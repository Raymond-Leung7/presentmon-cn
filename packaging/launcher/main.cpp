#include <windows.h>
#include <lm.h>
#include <sddl.h>
#include <shellapi.h>

#include <array>
#include <cwchar>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace
{
    constexpr wchar_t LauncherTitle[] = L"PresentMon CN Launcher";
    constexpr wchar_t RepairCaptureAccessArgument[] = L"--repair-capture-access";

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

    bool TryParseRepairCaptureAccessArgument(
        std::optional<std::wstring>& userSid,
        DWORD& error)
    {
        int argumentCount = 0;
        LPWSTR* argumentValues = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
        if (argumentValues == nullptr) {
            error = GetLastError();
            return false;
        }

        if (argumentCount >= 2 &&
            std::wcscmp(argumentValues[1], RepairCaptureAccessArgument) == 0) {
            if (argumentCount != 3 || argumentValues[2][0] == L'\0') {
                LocalFree(argumentValues);
                error = ERROR_BAD_ARGUMENTS;
                return false;
            }
            userSid = argumentValues[2];
        }

        LocalFree(argumentValues);
        return true;
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

    bool TryGetPerformanceLogUserMembership(bool& isMember, DWORD& error)
    {
        isMember = false;
        std::array<BYTE, SECURITY_MAX_SID_SIZE> sidBuffer{};
        DWORD sidSize = (DWORD)sidBuffer.size();
        if (!CreateWellKnownSid(
            WinBuiltinPerfLoggingUsersSid,
            nullptr,
            sidBuffer.data(),
            &sidSize)) {
            error = GetLastError();
            return false;
        }

        BOOL membership = FALSE;
        if (!CheckTokenMembership(nullptr, sidBuffer.data(), &membership)) {
            error = GetLastError();
            return false;
        }
        isMember = membership != FALSE;
        return true;
    }

    bool TryGetCurrentUserSid(std::wstring& userSid, DWORD& error)
    {
        HANDLE token = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
            error = GetLastError();
            return false;
        }

        DWORD tokenUserSize = 0;
        GetTokenInformation(token, TokenUser, nullptr, 0, &tokenUserSize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || tokenUserSize == 0) {
            error = GetLastError();
            CloseHandle(token);
            return false;
        }

        std::vector<BYTE> tokenUserBuffer(tokenUserSize);
        if (!GetTokenInformation(
            token,
            TokenUser,
            tokenUserBuffer.data(),
            tokenUserSize,
            &tokenUserSize)) {
            error = GetLastError();
            CloseHandle(token);
            return false;
        }
        CloseHandle(token);

        const auto* tokenUser = reinterpret_cast<const TOKEN_USER*>(tokenUserBuffer.data());
        LPWSTR sidString = nullptr;
        if (!ConvertSidToStringSidW(tokenUser->User.Sid, &sidString)) {
            error = GetLastError();
            return false;
        }
        userSid = sidString;
        LocalFree(sidString);
        return true;
    }

    bool TryGetPerformanceLogUsersGroupName(std::wstring& groupName, DWORD& error)
    {
        std::array<BYTE, SECURITY_MAX_SID_SIZE> sidBuffer{};
        DWORD sidSize = (DWORD)sidBuffer.size();
        if (!CreateWellKnownSid(
            WinBuiltinPerfLoggingUsersSid,
            nullptr,
            sidBuffer.data(),
            &sidSize)) {
            error = GetLastError();
            return false;
        }

        DWORD nameSize = 0;
        DWORD domainSize = 0;
        SID_NAME_USE sidType = SidTypeUnknown;
        LookupAccountSidW(
            nullptr,
            sidBuffer.data(),
            nullptr,
            &nameSize,
            nullptr,
            &domainSize,
            &sidType);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || nameSize == 0) {
            error = GetLastError();
            return false;
        }

        std::vector<wchar_t> nameBuffer(nameSize);
        std::vector<wchar_t> domainBuffer(domainSize == 0 ? 1 : domainSize);
        if (!LookupAccountSidW(
            nullptr,
            sidBuffer.data(),
            nameBuffer.data(),
            &nameSize,
            domainBuffer.data(),
            &domainSize,
            &sidType)) {
            error = GetLastError();
            return false;
        }

        groupName = nameBuffer.data();
        return true;
    }

    bool TryRepairCaptureAccess(const std::wstring& userSid, DWORD& error)
    {
        PSID sid = nullptr;
        if (!ConvertStringSidToSidW(userSid.c_str(), &sid)) {
            error = GetLastError();
            return false;
        }
        if (!IsValidSid(sid)) {
            LocalFree(sid);
            error = ERROR_INVALID_SID;
            return false;
        }

        std::wstring groupName;
        if (!TryGetPerformanceLogUsersGroupName(groupName, error)) {
            LocalFree(sid);
            return false;
        }

        LOCALGROUP_MEMBERS_INFO_0 memberInfo{};
        memberInfo.lgrmi0_sid = sid;
        const NET_API_STATUS status = NetLocalGroupAddMembers(
            nullptr,
            groupName.c_str(),
            0,
            reinterpret_cast<LPBYTE>(&memberInfo),
            1);
        LocalFree(sid);

        if (status != NERR_Success && status != ERROR_MEMBER_IN_ALIAS) {
            error = (DWORD)status;
            return false;
        }
        return true;
    }

    bool LaunchCaptureAccessRepair(
        const std::filesystem::path& executable,
        const std::filesystem::path& directory,
        const std::wstring& userSid,
        DWORD& error)
    {
        std::wstring parameters = RepairCaptureAccessArgument;
        parameters += L" ";
        parameters += userSid;

        SHELLEXECUTEINFOW executeInfo{};
        executeInfo.cbSize = (DWORD)sizeof(executeInfo);
        executeInfo.lpVerb = L"runas";
        executeInfo.lpFile = executable.c_str();
        executeInfo.lpParameters = parameters.c_str();
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

        std::optional<std::wstring> repairUserSid;
        if (!TryParseRepairCaptureAccessArgument(repairUserSid, error)) {
            ShowError(MakeWindowsErrorMessage(
                L"PresentMon could not parse the capture access repair request.", {}, error));
            return 5;
        }

        bool isElevated = false;
        if (!TryGetCurrentProcessElevation(isElevated, error)) {
            ShowError(MakeWindowsErrorMessage(
                L"PresentMon could not check the current capture permissions.", {}, error));
            return 5;
        }

        if (repairUserSid) {
            if (!isElevated) {
                MessageBoxW(nullptr,
                    L"\u4fee\u590d\u6355\u83b7\u6743\u9650\u9700\u8981\u7ba1\u7406\u5458\u6388\u6743\u3002\n\n"
                    L"\u8bf7\u666e\u901a\u542f\u52a8 PresentMon CN\uff0c\u7136\u540e\u5728\u6743\u9650\u63d0\u793a\u4e2d\u9009\u62e9\u201c\u662f\u201d\u3002",
                    LauncherTitle,
                    MB_OK | MB_ICONWARNING | MB_SETFOREGROUND);
                return 5;
            }

            if (!TryRepairCaptureAccess(*repairUserSid, error)) {
                ShowError(MakeWindowsErrorMessage(
                    L"PresentMon could not add the account to Performance Log Users.", {}, error));
                return 5;
            }

            MessageBoxW(nullptr,
                L"\u6355\u83b7\u6743\u9650\u5df2\u4fee\u590d\u3002\n\n"
                L"\u8bf7\u5148\u6ce8\u9500 Windows \u5e76\u91cd\u65b0\u767b\u5f55\uff0c\u7136\u540e\u76f4\u63a5\u53cc\u51fb PresentMon CN \u5feb\u6377\u65b9\u5f0f\u666e\u901a\u542f\u52a8\u3002\n\n"
                L"\u8bf7\u4e0d\u8981\u9009\u62e9\u201c\u4ee5\u7ba1\u7406\u5458\u8eab\u4efd\u8fd0\u884c\u201d\u3002",
                LauncherTitle,
                MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
            return 0;
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

        if (isElevated) {
            MessageBoxW(nullptr,
                L"PresentMon CN \u4e0d\u5e94\u4ee5\u7ba1\u7406\u5458\u8eab\u4efd\u8fd0\u884c\uff0c\u5426\u5219\u53ef\u80fd\u51fa\u73b0\u9ed1\u5c4f\u6216\u65e0\u6cd5\u6b63\u5e38\u9000\u51fa\u3002\n\n"
                L"\u8bf7\u5173\u95ed\u672c\u7a97\u53e3\uff0c\u7136\u540e\u76f4\u63a5\u53cc\u51fb\u684c\u9762\u6216\u5f00\u59cb\u83dc\u5355\u4e2d\u7684 PresentMon CN \u5feb\u6377\u65b9\u5f0f\u542f\u52a8\uff0c"
                L"\u4e0d\u8981\u9009\u62e9\u201c\u4ee5\u7ba1\u7406\u5458\u8eab\u4efd\u8fd0\u884c\u201d\u3002",
                LauncherTitle,
                MB_OK | MB_ICONWARNING | MB_SETFOREGROUND);
            return 5;
        }

        bool isPerformanceLogUser = false;
        if (!TryGetPerformanceLogUserMembership(isPerformanceLogUser, error)) {
            ShowError(MakeWindowsErrorMessage(
                L"PresentMon could not check Performance Log Users membership.", {}, error));
            return 5;
        }

        if (!isPerformanceLogUser) {
            const auto repairResponse = MessageBoxW(nullptr,
                L"\u5f53\u524d Windows \u8d26\u6237\u8fd8\u6ca1\u6709\u6355\u83b7\u6027\u80fd\u6570\u636e\u6240\u9700\u7684\u6743\u9650\u3002\n\n"
                L"PresentMon CN \u53ef\u4ee5\u8bf7\u6c42\u4e00\u6b21\u7ba1\u7406\u5458\u6388\u6743\uff0c\u4ec5\u5c06\u5f53\u524d\u8d26\u6237\u52a0\u5165"
                L"\u201cPerformance Log Users\uff08\u6027\u80fd\u65e5\u5fd7\u7528\u6237\uff09\u201d\u672c\u5730\u7ec4\u3002\n\n"
                L"\u4fee\u590d\u5b8c\u6210\u540e\u9700\u8981\u6ce8\u9500 Windows \u5e76\u91cd\u65b0\u767b\u5f55\u3002\u5728\u6b64\u4e4b\u524d PresentMon CN \u4e0d\u4f1a\u542f\u52a8\u3002\n\n"
                L"\u662f\u5426\u73b0\u5728\u4fee\u590d\u6355\u83b7\u6743\u9650\uff1f",
                LauncherTitle,
                MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING | MB_SETFOREGROUND);

            if (repairResponse == IDYES) {
                std::wstring userSid;
                if (!TryGetCurrentUserSid(userSid, error)) {
                    ShowError(MakeWindowsErrorMessage(
                        L"PresentMon could not identify the current Windows account.", {}, error));
                    return 5;
                }
                if (!LaunchCaptureAccessRepair(
                    launcherPath,
                    launcherDirectory,
                    userSid,
                    error)) {
                    if (error == ERROR_CANCELLED) {
                        MessageBoxW(nullptr,
                            L"\u7ba1\u7406\u5458\u6388\u6743\u5df2\u53d6\u6d88\uff0c\u6355\u83b7\u6743\u9650\u6ca1\u6709\u66f4\u6539\u3002",
                            LauncherTitle,
                            MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
                    }
                    else {
                        ShowError(MakeWindowsErrorMessage(
                            L"PresentMon could not start the capture access repair.",
                            launcherPath,
                            error));
                    }
                    return 5;
                }
                return 0;
            }
            return 5;
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
