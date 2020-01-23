set MAX_VERSION=2020
set "MAX_SDK_PATH=ADSK_3DSMAX_SDK_%MAX_VERSION%"

echo off
call set MAX_INCLUDE_DIR=%%%MAX_SDK_PATH%%%\include
call set MAX_LIB_DIR=%%%MAX_SDK_PATH%%%\lib\x64\Release
call set MAX_SAMPLES_DIR=%%%MAX_SDK_PATH%%%\samples
call set MAX_MORPHER_LIB_DIR=%%%MAX_SDK_PATH%%%\samples\modifiers\morpher\Lib\x64\Release

echo %MAX_MORPHER_LIB_DIR%\Morpher.lib
if not exist "%MAX_MORPHER_LIB_DIR%\Morpher.lib" (
    toolchain.bat
    msbuild "%MAX_SAMPLES_DIR%\modifiers\morpher\morpher.vcxproj" /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo /p:PlatformToolset=v141 /p:WindowsTargetPlatformVersion=%WindowsSDKVersion%
)

MeshSync_3dsMax.sln
