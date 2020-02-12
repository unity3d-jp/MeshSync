set MAYA_VERSION=2020
set "MAYA_SDK_PATH=MAYA_SDK_%MAYA_VERSION%"
set MAYA_LT=0

echo off
call set MAYA_INCLUDE_DIR=%%%MAYA_SDK_PATH%%%\include
call set MAYA_LIB_DIR=%%%MAYA_SDK_PATH%%%\lib

MeshSync_Maya.sln
