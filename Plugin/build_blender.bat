call buildtools.bat

msbuild PyMeshSync.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

mkdir "UnityMeshSync for Blender"
mkdir "UnityMeshSync for Blender\MeshSync"
copy _out\x64_Master\python\MeshSync\*.py "UnityMeshSync for Blender\MeshSync"
copy _out\x64_Master\python\MeshSync\*.pyd "UnityMeshSync for Blender\MeshSync"
copy MeshSyncClientBlender\* "UnityMeshSync for Blender"
del "UnityMeshSync for Blender.zip"
powershell.exe -nologo -noprofile -command "Compress-Archive -Path 'UnityMeshSync for Blender\*' -DestinationPath 'UnityMeshSync for Blender.zip'"
