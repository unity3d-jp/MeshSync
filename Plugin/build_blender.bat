call buildtools.bat

set PYTHON_VERSION=python3.5
set PYTHON_LIB_DIR=%cd%\External\python35\lib64
set PYTHON_INCLUDE_DIR=%cd%\External\python35\include
msbuild PyMeshSync.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

set PYTHON_VERSION=python3.6
set PYTHON_LIB_DIR=%cd%\External\python36\lib64
set PYTHON_INCLUDE_DIR=%cd%\External\python36\include
msbuild PyMeshSync.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

mkdir "UnityMeshSync for Blender\Blender2.7\MeshSync"
mkdir "UnityMeshSync for Blender\Blender2.8\MeshSync"
copy _out\x64_Master\python3.5\MeshSync\*.py "UnityMeshSync for Blender\Blender2.7\MeshSync"
copy _out\x64_Master\python3.5\MeshSync\*.pyd "UnityMeshSync for Blender\Blender2.7\MeshSync"
copy MeshSyncClientBlender\* "UnityMeshSync for Blender\Blender2.7"
copy _out\x64_Master\python3.6\MeshSync\*.py "UnityMeshSync for Blender\Blender2.8\MeshSync"
copy _out\x64_Master\python3.6\MeshSync\*.pyd "UnityMeshSync for Blender\Blender2.8\MeshSync"
copy MeshSyncClientBlender\* "UnityMeshSync for Blender\Blender2.8"
del "UnityMeshSync for Blender.zip"
powershell.exe -nologo -noprofile -command "Compress-Archive -Path 'UnityMeshSync for Blender\*' -DestinationPath 'UnityMeshSync for Blender.zip'"
