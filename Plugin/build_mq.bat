call buildtools.bat

msbuild MeshSyncClientMQ4.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
msbuild MeshSyncClientMQ4.vcxproj /t:Build /p:Configuration=Master /p:Platform=Win32 /m /nologo
msbuild MeshSyncClientMQ3.vcxproj /t:Build /p:Configuration=Master /p:Platform=Win32 /m /nologo

set DIST_DIR_MQ4_64="dist\UnityMeshSync_Metasequoia4_Windows_64bit"
set DIST_DIR_MQ4_32="dist\UnityMeshSync_Metasequoia4_Windows_32bit"
set DIST_DIR_MQ3="dist\UnityMeshSync_Metasequoia3_Windows"
mkdir "%DIST_DIR_MQ4_64%"
mkdir "%DIST_DIR_MQ4_32%"
mkdir "%DIST_DIR_MQ3%"
copy _out\x64_Master\MeshSyncClientMQ4.dll "%DIST_DIR_MQ4_64%"
copy _out\Win32_Master\MeshSyncClientMQ4.dll "%DIST_DIR_MQ4_32%"
copy _out\Win32_Master\MeshSyncClientMQ3.dll "%DIST_DIR_MQ3%"
