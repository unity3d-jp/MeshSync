call buildtools.bat

set BLENDER_VERSION=blender2.79
set BLENDER_INCLUDE_DIR=%cd%\External\blender-2.79
set PYTHON_VERSION=python3.5
set PYTHON_LIB_DIR=%cd%\External\python35\lib64
set PYTHON_INCLUDE_DIR=%cd%\External\python35\include
msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

set BLENDER_VERSION=blender2.80
set BLENDER_INCLUDE_DIR=%cd%\External\blender-2.80
set PYTHON_VERSION=python3.6
set PYTHON_LIB_DIR=%cd%\External\python36\lib64
set PYTHON_INCLUDE_DIR=%cd%\External\python36\include
msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

mkdir "UnityMeshSync for Blender2.79\MeshSyncClientBlender"
mkdir "UnityMeshSync for Blender2.80\MeshSyncClientBlender"
xcopy /YQE MeshSyncClientBlender\python "UnityMeshSync for Blender2.79"
xcopy /YQE MeshSyncClientBlender\python "UnityMeshSync for Blender2.80"
copy _out\x64_Master\python3.5\*.pyd "UnityMeshSync for Blender2.79\MeshSyncClientBlender"
copy _out\x64_Master\python3.6\*.pyd "UnityMeshSync for Blender2.80\MeshSyncClientBlender"
del "UnityMeshSync for Blender.zip"
powershell.exe -nologo -noprofile -command "Compress-Archive -Path 'UnityMeshSync for Blender2.79\*' -DestinationPath 'UnityMeshSync Blender2.79.zip'"
powershell.exe -nologo -noprofile -command "Compress-Archive -Path 'UnityMeshSync for Blender2.80\*' -DestinationPath 'UnityMeshSync Blender2.80.zip'"
powershell.exe -nologo -noprofile -command "Compress-Archive -Path 'UnityMeshSync Blender*.zip' -DestinationPath 'UnityMeshSync for Blender.zip'"
del "UnityMeshSync Blender*.zip"