call toolchain.bat

msbuild MeshSyncClientXismo.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
IF %ERRORLEVEL% NEQ 0 (
    pause
    exit /B 1
)

msbuild MeshSyncClientXismoHook.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
IF %ERRORLEVEL% NEQ 0 (
    pause
    exit /B 1
)

set DIST_DIR="dist\UnityMeshSync_xismo_Windows"
mkdir "%DIST_DIR%"
copy _out\x64_Master\MeshSyncClientXismo.exe "%DIST_DIR%"
copy _out\x64_Master\MeshSyncClientXismoHook.dll "%DIST_DIR%"
