call toolchain.bat
call :Build 2.79 35
call :Build 2.80 37
exit /B 0

:Build
    set BLENDER_VERSION=%~1
    set BLENDER_PYTHON_VERSION=%~2
    set BLENDER_MODULE_BLENDER_VERSION=%BLENDER_VERSION:.=_%
    set BLENDER_INCLUDE_DIRS=^
%cd%\External\blender-%BLENDER_VERSION%\include\blenkernel;^
%cd%\External\blender-%BLENDER_VERSION%\include\blenlib;^
%cd%\External\blender-%BLENDER_VERSION%\include\bmesh;^
%cd%\External\blender-%BLENDER_VERSION%\include\makesdna;^
%cd%\External\blender-%BLENDER_VERSION%\include\makesrna;^
%cd%\External\blender-%BLENDER_VERSION%\include\python;
    set BLENDER_PYTHON_INCLUDE_DIR=%cd%\External\python%BLENDER_PYTHON_VERSION%\include
    set BLENDER_PYTHON_LIB_DIR=%cd%\External\python%BLENDER_PYTHON_VERSION%\lib64
    msbuild MeshSyncClientBlender.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )

    set DIST_DIR="dist\UnityMeshSync_Blender_Windows\blender-%BLENDER_VERSION%"
    set PYD_DIR="%DIST_DIR%\MeshSyncClientBlender"
    xcopy /Y MeshSyncClientBlender\python\unity_mesh_sync_common.py "%DIST_DIR%\"
    xcopy /Y MeshSyncClientBlender\python\%BLENDER_VERSION%\unity_mesh_sync.py "%DIST_DIR%\"
    xcopy /Y MeshSyncClientBlender\python\__init__.py "%PYD_DIR%\"
    xcopy /Y _out\x64_Release\MeshSyncClientBlender-%BLENDER_VERSION%\*.pyd "%PYD_DIR%\"
    exit /B 0
