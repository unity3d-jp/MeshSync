call buildtools.bat
call :Build 2.79 35
@rem call :Build 2.8 36
exit /B 0

:Build
    set BLENDER_VERSION=%~1
    set PYTHON_VERSION=%~2
    set BLENDER_INCLUDE_DIR="%cd%\External\blender-%BLENDER_VERSION%"
    set PYTHON_LIB_DIR="%cd%\External\python%PYTHON_VERSION%\lib64"
    set PYTHON_INCLUDE_DIR="%cd%\External\python%PYTHON_VERSION%\include"
    set DIST_DIR="dist\UnityMeshSync_blender-%BLENDER_VERSION%_Windows"
    msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
    xcopy /YS MeshSyncClientBlender\python "%DIST_DIR%\"
    xcopy /Y _out\x64_Master\MeshSyncClientBlender-%BLENDER_VERSION%\*.pyd "%DIST_DIR%\MeshSyncClientBlender\"
    exit /B 0
