call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"

msbuild MeshSyncClientXismo.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
msbuild MeshSyncClientXismoHook.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

mkdir "UnityMeshSync for Xismo"
copy _out\x64_Master\MeshSyncClientXismo.exe "UnityMeshSync for Xismo"
copy _out\x64_Master\MeshSyncClientXismoHook.dll "UnityMeshSync for Xismo"
del "UnityMeshSync for Xismo.zip"
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('UnityMeshSync for Xismo', 'UnityMeshSync for Xismo.zip'); }"
