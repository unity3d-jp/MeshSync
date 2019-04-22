call toolchain.bat
call :BuildMQ4 .64 mqsdk464
call :BuildMQ4 .7beta mqsdk47_beta3
call :BuildMQ3
exit /B 0

:BuildMQ3
    msbuild MeshSyncClientMQ3.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )
    set DIST_DIR_MQ3="dist\UnityMeshSync_Metasequoia_Windows\Metasequoia3"
    mkdir "%DIST_DIR_MQ3%"
    copy _out\Win32_Release\MeshSyncClientMQ3\MeshSyncClientMQ3.dll "%DIST_DIR_MQ3%"
    exit /B 0

:BuildMQ4
    set MQ4_VERSION=%~1
    set MQ4_SDK_DIR=%cd%\External\%~2\mqsdk
    msbuild MeshSyncClientMQ4.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )
    msbuild MeshSyncClientMQ4.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )
    set DIST_DIR_MQ4_64="dist\UnityMeshSync_Metasequoia_Windows\Metasequoia4%MQ4_VERSION%_64bit"
    set DIST_DIR_MQ4_32="dist\UnityMeshSync_Metasequoia_Windows\Metasequoia4%MQ4_VERSION%_32bit"
    mkdir "%DIST_DIR_MQ4_64%"
    mkdir "%DIST_DIR_MQ4_32%"
    copy _out\x64_Release\MeshSyncClientMQ4%MQ4_VERSION%\MeshSyncClientMQ4.dll "%DIST_DIR_MQ4_64%"
    copy _out\Win32_Release\MeshSyncClientMQ4%MQ4_VERSION%\MeshSyncClientMQ4.dll "%DIST_DIR_MQ4_32%"
    exit /B 0