call toolchain.bat

call :Build 2019 5.9.3
exit /B 0

:Build
    set VRED_VERSION=%~1
    set VRED_QT_VERSION=%~2
    set VRED_QT_DIR=%cd%\External\Qt\%VRED_QT_VERSION%\msvc2017_64
    msbuild MeshSyncClientVRED.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )
    msbuild MeshSyncClientVREDHook.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )

    set DIST_DIR="dist\UnityMeshSync_VRED_Windows"
    set CONTENT_DIR="%DIST_DIR%\VRED%VRED_VERSION%"
    xcopy /Y _out\x64_Release\MeshSyncClientVRED%VRED_VERSION%\MeshSyncClientVRED.exe "%CONTENT_DIR%\"
    xcopy /Y _out\x64_Release\MeshSyncClientVRED%VRED_VERSION%\MeshSyncClientVREDHook.dll "%CONTENT_DIR%\"
    xcopy /Y MeshSyncClientVRED\%VRED_VERSION%\*.bat "%CONTENT_DIR%\"
    exit /B 0
