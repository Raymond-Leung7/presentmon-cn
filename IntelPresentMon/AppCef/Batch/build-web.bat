@echo off
setlocal
pushd "%~dp0..\ipm-ui-vue" || exit /b 1

echo "Install NPM Packages..."
call npm ci
if errorlevel 1 goto :fail

echo "Build SPA..."
call npm run build
if errorlevel 1 goto :fail

popd
exit /b 0

:fail
set "buildWebExitCode=%errorlevel%"
popd
exit /b %buildWebExitCode%
