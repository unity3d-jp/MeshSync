call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"

msbuild MeshSyncClientMQ3.vcxproj /t:Build /p:Configuration=Master /p:Platform=Win32 /m /nologo
msbuild MeshSyncClientMQ4.vcxproj /t:Build /p:Configuration=Master /p:Platform=Win32 /m /nologo
msbuild MeshSyncClientMQ4.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

mkdir "UnityMeshSync for Metasequoia\Metasequoia3\Plugins\Station"
mkdir "UnityMeshSync for Metasequoia\Metasequoia4 (32bit)\Plugins\Station"
mkdir "UnityMeshSync for Metasequoia\Metasequoia4 (64bit)\Plugins\Station"
copy _out\MeshSyncClientMQ3_Win32_Master\MeshSyncClientMQ3.dll "UnityMeshSync for Metasequoia\Metasequoia3\Plugins\Station"
copy _out\MeshSyncClientMQ4_Win32_Master\MeshSyncClientMQ4.dll "UnityMeshSync for Metasequoia\Metasequoia4 (32bit)\Plugins\Station"
copy _out\MeshSyncClientMQ4_x64_Master\MeshSyncClientMQ4.dll "UnityMeshSync for Metasequoia\Metasequoia4 (64bit)\Plugins\Station"
del "UnityMeshSync for Metasequoia.zip"
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('UnityMeshSync for Metasequoia', 'UnityMeshSync for Metasequoia.zip'); }"
