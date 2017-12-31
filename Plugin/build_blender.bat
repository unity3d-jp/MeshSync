call buildtools.bat

set PYTHON_VERSION=python3.5
set PYTHON_LIB_DIR=%cd%\External\python35\lib64
set PYTHON_INCLUDE_DIR=%cd%\External\python35\include
msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

set PYTHON_VERSION=python3.6
set PYTHON_LIB_DIR=%cd%\External\python36\lib64
set PYTHON_INCLUDE_DIR=%cd%\External\python36\include
msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

mkdir "UnityMeshSync for Blender\Blender2.7\MeshSyncClientBlender"
mkdir "UnityMeshSync for Blender\Blender2.8\MeshSyncClientBlender"
xcopy /YQE MeshSyncClientBlender\python "UnityMeshSync for Blender\Blender2.7"
xcopy /YQE MeshSyncClientBlender\python "UnityMeshSync for Blender\Blender2.8"
copy _out\x64_Master\python3.5\*.pyd "UnityMeshSync for Blender\Blender2.7\MeshSyncClientBlender"
copy _out\x64_Master\python3.6\*.pyd "UnityMeshSync for Blender\Blender2.8\MeshSyncClientBlender"
del "UnityMeshSync for Blender.zip"
powershell.exe -nologo -noprofile -command "Compress-Archive -Path 'UnityMeshSync for Blender\*' -DestinationPath 'UnityMeshSync for Blender.zip'"
