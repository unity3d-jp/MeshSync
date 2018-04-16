call buildtools.bat

msbuild MeshSyncClientXismo.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
msbuild MeshSyncClientXismoHook.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

set DIST_DIR="dist\xismo\UnityMeshSync_xismo_Windows"
mkdir "%DIST_DIR%"
copy _out\x64_Master\MeshSyncClientXismo.exe "%DIST_DIR%"
copy _out\x64_Master\MeshSyncClientXismoHook.dll "%DIST_DIR%"
