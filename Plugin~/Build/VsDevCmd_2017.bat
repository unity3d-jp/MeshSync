for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" /version [15.0^,16^) /property installationPath`) do (
  set VSDIR=%%i
  echo %%i
)
call "%VSDIR%\Common7\Tools\VsDevCmd.bat"
cd %~dp0
