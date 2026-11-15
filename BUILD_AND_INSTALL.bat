@echo off
setlocal
cd /d "%~dp0"
where cmake >nul 2>&1
if errorlevel 1 goto missing_cmake
where git >nul 2>&1
if errorlevel 1 goto missing_git

echo === Ritual: configure (downloads JUCE on first build) ===
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto failed

echo === Ritual: compile VST3 ===
cmake --build build --config Release --target Ritual_VST3 --parallel 4
if errorlevel 1 goto failed

set "PLUGIN=%CD%\build\Ritual_artefacts\Release\VST3\Ritual.vst3"
if not exist "%PLUGIN%" goto missing_plugin
set "DEST=%LOCALAPPDATA%\Programs\Common\VST3"
if not exist "%DEST%" mkdir "%DEST%"
xcopy /E /I /Y "%PLUGIN%" "%DEST%\Ritual.vst3\" >nul
if errorlevel 1 goto failed

echo.
echo Ritual was built and copied to:
echo %DEST%\Ritual.vst3
echo.
echo IMPORTANT: FL Studio may not scan the user-specific directory above.
echo If Ritual is not detected, manually copy Ritual.vst3 to:
echo C:\Program Files\Common Files\VST3

echo Then open FL Studio ^> Options ^> Manage plugins ^> Find installed plugins.
pause
exit /b 0
:missing_cmake
echo ERROR: Install CMake and ensure it's on PATH.
goto failed
:missing_git
echo ERROR: Install Git for Windows and ensure it's on PATH.
goto failed
:missing_plugin
echo ERROR: No VST3 bundle found in the expected build output.
goto failed
:failed
echo.
echo Build did not finish. Check README.md and the error shown above.
pause
exit /b 1
