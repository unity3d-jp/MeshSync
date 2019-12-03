setlocal

cd /d "%~dp0"
call toolchain.bat
call :Build 2020
call :Build 2019
call :Build 2018
call :Build 2017
call :Build 2016
exit /B 0

:Build
    set MAX_VERSION=%~1
    set "INCLUDE_VAR=MAX_INCLUDE_DIR_%MAX_VERSION%"
    set "LIB_VAR=MAX_LIB_DIR_%MAX_VERSION%"
    
    call set MAX_INCLUDE_DIR=%%%INCLUDE_VAR%%%
    call set MAX_LIB_DIR=%%%LIB_VAR%%%
       
    msbuild MeshSyncClient3dsMax.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )
    
    set DIST_DIR="dist\UnityMeshSync_3dsMax_Windows"
    set CONTENT_DIR="%DIST_DIR%\3dsMax%MAX_VERSION%"
    xcopy /Y _out\x64_Release\MeshSyncClient3dsMax%MAX_VERSION%\MeshSyncClient3dsMax.dlu "%CONTENT_DIR%\"
    exit /B 0