call buildtools.bat

msbuild MeshSyncClientXismo.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
msbuild MeshSyncClientXismoHook.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

mkdir "UnityMeshSync for xismo"
copy _out\x64_Master\MeshSyncClientXismo.exe "UnityMeshSync for xismo"
copy _out\x64_Master\MeshSyncClientXismoHook.dll "UnityMeshSync for xismo"
del "UnityMeshSync for xismo.zip"
powershell.exe -nologo -noprofile -command "Compress-Archive -Path 'UnityMeshSync for xismo\*' -DestinationPath 'UnityMeshSync for xismo.zip'"
