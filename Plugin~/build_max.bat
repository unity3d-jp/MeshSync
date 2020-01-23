setlocal

cd /d "%~dp0"
call toolchain.bat
call :Build 2020
call :Build 2019
call :Build 2018
call :Build 2017
rem call :Build 2016
exit /B 0

:Build
    set MAX_VERSION=%~1
    set "MAX_SDK_PATH=MAX_SDK_%MAX_VERSION%"
    
    call set MAX_INCLUDE_DIR=%%%MAX_SDK_PATH%%%\include
    call set MAX_LIB_DIR=%%%MAX_SDK_PATH%%%\lib\x64\Release
          
    msbuild MeshSyncClient3dsMax.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
    IF %ERRORLEVEL% NEQ 0 (
        pause
        exit /B 1
    )
    
    set DIST_DIR="dist\UnityMeshSync_3dsMax_Windows"
    set CONTENT_DIR="%DIST_DIR%\3dsMax%MAX_VERSION%"
    xcopy /Y _out\x64_Release\MeshSyncClient3dsMax%MAX_VERSION%\MeshSyncClient3dsMax.dlu "%CONTENT_DIR%\"
    exit /B 0