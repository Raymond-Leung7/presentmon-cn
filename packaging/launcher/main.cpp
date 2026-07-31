#include <windows.h>
#include <shellapi.h>

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    constexpr wchar_t LauncherTitle[] = L"PresentMon CN Launcher";

    void ShowError(const std::wstring& message) noexcept
    {
        MessageBoxW(nullptr, message.c_str(), LauncherTitle,
            MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    }

    std::wstring FormatWindowsError(DWORD error)
    {
        wchar_t* rawMessage = nullptr;
        const DWORD length = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPWSTR>(&rawMessage),
            0,
            nullptr);

        std::wstring message;
        if (length != 0 && rawMessage != nullptr) {
            message.assign(rawMessage, length);
            LocalFree(rawMessage);
            while (!message.empty() &&
                (message.back() == L'\r' || message.back() == L'\n' || message.back() == L' ')) {
                message.pop_back();
            }
        }
        else {
            message = L"Unknown Windows error";
        }

        return message;
    }

    std::wstring MakeWindowsErrorMessage(
        const std::wstring& summary,
        const std::filesystem::path& path,
        DWORD error)
    {
        std::wstring message = summary;
        if (!path.empty()) {
            message += L"\n\nPath:\n";
            message += path.wstring();
        }
        message += L"\n\nWindows error ";
        message += std::to_wstring(error);
        message += L": ";
        message += FormatWindowsError(error);
        return message;
    }

    bool GetLauncherPaths(
        std::filesystem::path& executable,
        std::filesystem::path& directory,
        DWORD& error)
    {
        std::vector<wchar_t> buffer(32768);
        SetLastError(ERROR_SUCCESS);
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), (DWORD)buffer.size());
        if (length == 0) {
            error = GetLastError();
            return false;
        }
        if (length >= (DWORD)buffer.size()) {
            error = ERROR_INSUFFICIENT_BUFFER;
            return false;
        }

        executable = std::filesystem::path{ std::wstring{ buffer.data(), length } };
        directory = executable.parent_path();
        if (directory.empty()) {
            error = ERROR_PATH_NOT_FOUND;
            return false;
        }
        return true;
    }

    bool IsFile(const std::filesystem::path& path)
    {
        const DWORD attributes = GetFileAttributesW(path.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    bool Exists(const std::filesystem::path& path)
    {
        return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
    }

    bool TryGetCurrentProcessElevation(bool& elevated, DWORD& error)
    {
        elevated = false;
        HANDLE token = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
            error = GetLastError();
            return false;
        }

        TOKEN_ELEVATION elevation{};
        DWORD returnedSize = 0;
        const BOOL success = GetTokenInformation(
            token,
            TokenElevation,
            &elevation,
            (DWORD)sizeof(elevation),
            &returnedSize);
        if (!success) {
            error = GetLastError();
            CloseHandle(token);
            return false;
        }
        CloseHandle(token);
        elevated = elevation.TokenIsElevated != 0;
        return true;
    }

    bool IsPerformanceLogUser()
    {
        std::array<BYTE, SECURITY_MAX_SID_SIZE> sidBuffer{};
        DWORD sidSize = (DWORD)sidBuffer.size();
        if (!CreateWellKnownSid(
            WinBuiltinPerfLoggingUsersSid,
            nullptr,
            sidBuffer.data(),
            &sidSize)) {
            return false;
        }

        BOOL isMember = FALSE;
        return CheckTokenMembership(nullptr, sidBuffer.data(), &isMember) && isMember;
    }

    bool RelaunchElevated(
        const std::filesystem::path& executable,
        const std::filesystem::path& directory,
        const wchar_t* arguments,
        DWORD& error)
    {
        SHELLEXECUTEINFOW executeInfo{};
        executeInfo.cbSize = (DWORD)sizeof(executeInfo);
        executeInfo.lpVerb = L"runas";
        executeInfo.lpFile = executable.c_str();
        executeInfo.lpParameters = arguments != nullptr && arguments[0] != L'\0'
            ? arguments
            : nullptr;
        executeInfo.lpDirectory = directory.c_str();
        executeInfo.nShow = SW_SHOWNORMAL;
        if (!ShellExecuteExW(&executeInfo)) {
            error = GetLastError();
            return false;
        }
        return true;
    }

    std::wstring FindMissingFiles(const std::filesystem::path& appDirectory)
    {
        constexpr std::array RequiredFiles{
            L"PresentMon.exe",
            L"PresentMonUI.exe",
            L"PresentMonService.exe",
            L"PresentMonAPI2.dll",
            L"PresentMonAPI2Loader.dll",
            L"TargetBlockList.txt",
        };

        std::wstring missing;
        for (const auto* fileName : RequiredFiles) {
            if (!IsFile(appDirectory / fileName)) {
                missing += L"  - app\\";
                missing += fileName;
                missing += L"\n";
            }
        }
        return missing;
    }
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR arguments, int)
{
    try {
        DWORD error = ERROR_SUCCESS;
        std::filesystem::path launcherPath;
        std::filesystem::path launcherDirectory;
        if (!GetLauncherPaths(launcherPath, launcherDirectory, error)) {
            ShowError(MakeWindowsErrorMessage(
                L"PresentMon could not locate the launcher directory.", {}, error));
            return 1;
        }

        const auto appDirectory = launcherDirectory / L"app";
        const auto presentMonPath = appDirectory / L"PresentMon.exe";
        const auto missingFiles = FindMissingFiles(appDirectory);
        if (!missingFiles.empty()) {
            std::wstring message =
                L"PresentMon cannot start because its application files are incomplete.\n\n"
                L"Missing required files:\n";
            message += missingFiles;
            message += L"\nRepair the installation or re-extract the portable package, then try again.";
            ShowError(message);
            return 2;
        }

        bool isElevated = false;
        if (!TryGetCurrentProcessElevation(isElevated, error)) {
            ShowError(MakeWindowsErrorMessage(
                L"PresentMon could not check the current capture permissions.", {}, error));
            return 5;
        }

        if (!isElevated && !IsPerformanceLogUser()) {
            if (!RelaunchElevated(launcherPath, launcherDirectory, arguments, error)) {
                ShowError(MakeWindowsErrorMessage(
                    L"PresentMon needs administrator permission to capture performance data.",
                    launcherPath,
                    error));
                return 5;
            }
            return 0;
        }

        std::wstring commandLine = L"\"";
        commandLine += presentMonPath.wstring();
        commandLine +=
            L"\" --svc-as-child"
            L" --control-pipe \\\\.\\pipe\\pm-cn-ctrl"
            L" --shm-name-prefix pm-cn-child-shm"
            L" --etw-session-name pm-cn-child-etw-session"
            L" --ui-mutex-name PresentMonCnUiBrowserProcess"
            L" --middleware-dll-path \".\\PresentMonAPI2.dll\"";

        if (Exists(launcherDirectory / L"portable.mode")) {
            commandLine += L" --files-working";
        }
        if (arguments != nullptr && arguments[0] != L'\0') {
            commandLine += L" ";
            commandLine += arguments;
        }

        std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
        mutableCommandLine.push_back(L'\0');

        STARTUPINFOW startupInfo{};
        startupInfo.cb = (DWORD)sizeof(startupInfo);
        PROCESS_INFORMATION processInfo{};
        if (!CreateProcessW(
            presentMonPath.c_str(),
            mutableCommandLine.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_UNICODE_ENVIRONMENT,
            nullptr,
            appDirectory.c_str(),
            &startupInfo,
            &processInfo)) {
            error = GetLastError();
            ShowError(MakeWindowsErrorMessage(
                L"PresentMon failed to start.", presentMonPath, error));
            return 3;
        }

        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return 0;
    }
    catch (...) {
        ShowError(
            L"PresentMon failed to prepare its launch command.\n\n"
            L"Re-extract the portable package and try again.");
        return 4;
    }
}
