setlocal
cd /d "%~dp0"
call toolchain.bat

msbuild mscore.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
IF %ERRORLEVEL% NEQ 0 (
    pause
    exit /B 1
)
