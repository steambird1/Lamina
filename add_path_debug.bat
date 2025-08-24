@echo off

set "PROJECT_ROOT=%~dp0"
set "DEBUG_DIR=%PROJECT_ROOT%cmake-build-debug"
if not exist "%DEBUG_DIR%" (
    echo Dir is not exist！path:%DEBUG_DIR%
    pause
    exit /b 1
)

:: 2. 检查路径是否已在 PATH 中（避免重复添加）
echo Checking path is in PATH...
echo %PATH% | findstr /i /c:"%DEBUG_DIR%" > nul 2>&1
if %errorLevel% equ 0 (
    echo path is in PATH%DEBUG_DIR%
    pause
    exit /b 0
)

:: 3. 添加路径到用户级 PATH（setx 无需管理员，当前用户生效）
echo Adding path to PATH:%DEBUG_DIR%
setx PATH "%PATH%;%DEBUG_DIR%"

:: 4. 校验是否添加成功
if %errorLevel% equ 0 (
    echo Successful
    echo please reopen the console
) else (
    echo Error!
    echo Add path to PATH manually
)

pause
exit /b 0