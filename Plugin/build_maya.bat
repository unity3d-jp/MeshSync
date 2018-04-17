call buildtools.bat
call :Build 2016.5
call :Build 2017
call :Build 2018
exit /B 0

:Build
    set MAYA_VERSION=%~1
    set MAYA_LIB_DIR=%cd%\External\Maya%MAYA_VERSION%\lib
    set MAYA_INCLUDE_DIR=%cd%\External\Maya%MAYA_VERSION%\include
    msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo

    set DIST_DIR="dist\UnityMeshSync_maya%MAYA_VERSION%_Windows"
    xcopy /Y MeshSyncClientMaya\MEL\*.mod "%DIST_DIR%\"
    xcopy /Y MeshSyncClientMaya\MEL\*.mel "%DIST_DIR%\UnityMeshSync\scripts\"
    xcopy /Y _out\x64_Master\MeshSyncClientMaya%MAYA_VERSION%\MeshSyncClientMaya.mll "%DIST_DIR%\UnityMeshSync\plug-ins\"
    exit /B 0