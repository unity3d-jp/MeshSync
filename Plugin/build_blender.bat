call buildtools.bat

set BLENDER_VERSION=2.79
set BLENDER_INCLUDE_DIR=%cd%\External\blender-2.79
set PYTHON_LIB_DIR=%cd%\External\python35\lib64
set PYTHON_INCLUDE_DIR=%cd%\External\python35\include
set DIST_DIR="dist\Blender\UnityMeshSync_blender-%BLENDER_VERSION%_Windows"
msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "%DIST_DIR%\MeshSyncClientBlender"
xcopy /YQE MeshSyncClientBlender\python "%DIST_DIR%"
copy _out\x64_Master\MeshSyncClientBlender-%BLENDER_VERSION%\*.pyd "%DIST_DIR%\MeshSyncClientBlender"

@rem set BLENDER_VERSION=blender2.8
@rem set BLENDER_INCLUDE_DIR=%cd%\External\blender-2.80
@rem set PYTHON_LIB_DIR=%cd%\External\python36\lib64
@rem set PYTHON_INCLUDE_DIR=%cd%\External\python36\include
@rem DIST_DIR="dist\Blender\UnityMeshSync_blender-%BLENDER_VERSION%_Windows"
@rem msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
@rem mkdir "%DIST_DIR%\MeshSyncClientBlender"
@rem xcopy /YQE MeshSyncClientBlender\python "%DIST_DIR%"
@rem copy _out\x64_Master\MeshSyncClientBlender-%BLENDER_VERSION%\*.pyd "%DIST_DIR%\MeshSyncClientBlender"
