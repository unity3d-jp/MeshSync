call buildtools.bat

set BLENDER_VERSION=blender2.79
set BLENDER_INCLUDE_DIR=%cd%\External\blender-2.79
set PYTHON_VERSION=python3.5
set PYTHON_LIB_DIR=%cd%\External\python35\lib64
set PYTHON_INCLUDE_DIR=%cd%\External\python35\include
msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

@rem set BLENDER_VERSION=blender2.80
@rem set BLENDER_INCLUDE_DIR=%cd%\External\blender-2.80
@rem set PYTHON_VERSION=python3.6
@rem set PYTHON_LIB_DIR=%cd%\External\python36\lib64
@rem set PYTHON_INCLUDE_DIR=%cd%\External\python36\include
@rem msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

mkdir "UnityMeshSync for Blender\MeshSyncClientBlender"
xcopy /YQE MeshSyncClientBlender\python "UnityMeshSync for Blender"
copy _out\x64_Master\python3.5\*.pyd "UnityMeshSync for Blender\MeshSyncClientBlender"

del "UnityMeshSync for Blender.zip"
powershell.exe -nologo -noprofile -command "Compress-Archive -Path 'UnityMeshSync for Blender\*' -DestinationPath 'UnityMeshSync Blender.zip'"
