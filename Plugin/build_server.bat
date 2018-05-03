call toolchain.bat

msbuild MeshSyncServer.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
IF %ERRORLEVEL% NEQ 0 (
    pause
    exit /B 1
)
