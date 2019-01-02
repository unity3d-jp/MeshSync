call toolchain.bat
call :Build 2018
call :Build 2017
call :Build 2016.5
call :Build 2016
call :Build 2015
call :Build 2018 LT
exit /B 0

:Build
    set MAYA_VERSION=%~2%~1
    echo target: %MAYA_VERSION%
    echo %MAYA_VERSION% | findstr /C:"LT">nul && (set MAYA_LT=1) || (set MAYA_LT=0)
    set MAYA_LIB_DIR=%cd%\External\Maya%~2%~1\lib
    set MAYA_INCLUDE_DIR=%cd%\External\Maya%~1\include
    msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )
    
    set DIST_DIR="dist\UnityMeshSync_Maya%~2_Windows\modules"
    set CONTENT_DIR="%DIST_DIR%\UnityMeshSync\%~1%~2"
    xcopy /Y MeshSyncClientMaya\MEL\*.mod "%DIST_DIR%\"
    xcopy /Y MeshSyncClientMaya\MEL\*.mel "%CONTENT_DIR%\scripts\"
    xcopy /Y _out\x64_Release\MeshSyncClientMaya%MAYA_VERSION%\MeshSyncClientMaya.mll "%CONTENT_DIR%\plug-ins\"
    exit /B 0
