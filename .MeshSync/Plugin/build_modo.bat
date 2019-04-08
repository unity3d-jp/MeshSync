call toolchain.bat
call :Build LXSDK_525410
exit /B 0

:Build
    set MODO_QT_VERSION=4.8.5
    set MODO_QT_DIR=%cd%\External\Qt\%MODO_QT_VERSION%
    set MODO_SDK_DIR=%~1
    set MODO_INCLUDE_DIR=%cd%\External\%MODO_SDK_DIR%\include
    set MODO_SOURCE_DIR=%cd%\External\%MODO_SDK_DIR%\common

    msbuild MeshSyncClientModo.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )
    
    set DIST_DIR="dist\UnityMeshSync_Modo_Windows"
    xcopy /Y _out\x64_Release\MeshSyncClientModo\MeshSyncClientModo.lx "%DIST_DIR%\"
    exit /B 0
