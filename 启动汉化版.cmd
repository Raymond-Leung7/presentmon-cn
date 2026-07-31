@echo off
setlocal

set "APP_DIR=%~dp0build\Debug"
set "APP_EXE=%APP_DIR%\PresentMon.exe"
set "MIDDLEWARE_DLL=%APP_DIR%\PresentMonAPI2.dll"

if not exist "%APP_EXE%" (
    echo PresentMon.exe was not found in "%APP_DIR%".
    pause
    exit /b 1
)

if not exist "%MIDDLEWARE_DLL%" (
    echo PresentMonAPI2.dll was not found in "%APP_DIR%".
    pause
    exit /b 1
)

pushd "%APP_DIR%"
start "" "%APP_EXE%" --svc-as-child --files-working --log-level verbose --log-svc-pipe-enable --middleware-dll-path "%MIDDLEWARE_DLL%" --log-middleware-copy
set "START_RESULT=%ERRORLEVEL%"
popd

if not "%START_RESULT%"=="0" (
    echo PresentMon failed to start with exit code %START_RESULT%.
    pause
)

exit /b %START_RESULT%
