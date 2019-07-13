call toolchain.bat

call :Build 190 5.6.2
exit /B 0


:Build
    set XISMO_VERSION=%~1
    set XISMO_QT_VERSION=%~2
    set XISMO_QT_DIR=%cd%\External\Qt\%XISMO_QT_VERSION%\msvc2015_64

    msbuild MeshSyncClientXismo.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )

    msbuild MeshSyncClientXismoHook.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )

    set DIST_DIR="dist\UnityMeshSync_xismo_Windows"
    set CONTENT_DIR="%DIST_DIR%\xismo%XISMO_VERSION%"
    xcopy /Y _out\x64_Release\MeshSyncClientXismo%XISMO_VERSION%\MeshSyncClientXismo.exe "%CONTENT_DIR%\"
    xcopy /Y _out\x64_Release\MeshSyncClientXismo%XISMO_VERSION%\MeshSyncClientXismoHook.dll "%CONTENT_DIR%\"
