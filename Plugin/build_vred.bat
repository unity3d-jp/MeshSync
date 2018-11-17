call toolchain.bat

msbuild MeshSyncClientVRED.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
IF %ERRORLEVEL% NEQ 0 (
    pause
    exit /B 1
)

msbuild MeshSyncClientVREDHook.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
IF %ERRORLEVEL% NEQ 0 (
    pause
    exit /B 1
)

set DIST_DIR="dist\UnityMeshSync_VRED_Windows"
mkdir "%DIST_DIR%"
copy _out\x64_Master\MeshSyncClientVRED.exe "%DIST_DIR%"
copy _out\x64_Master\MeshSyncClientVREDHook.dll "%DIST_DIR%"
