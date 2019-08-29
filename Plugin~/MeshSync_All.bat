set MAX_VERSION=2020
set MAX_LIB_DIR=%cd%\External\3ds Max %MAX_VERSION% SDK\maxsdk\lib\x64\Release
set MAX_INCLUDE_DIR=%cd%\External\3ds Max %MAX_VERSION% SDK\maxsdk\include

set BLENDER_VERSION=2.80
set BLENDER_PYTHON_VERSION=37
set BLENDER_INCLUDE_DIRS=^
%cd%\External\blender-%BLENDER_VERSION%\include\blenkernel;^
%cd%\External\blender-%BLENDER_VERSION%\include\blenlib;^
%cd%\External\blender-%BLENDER_VERSION%\include\bmesh;^
%cd%\External\blender-%BLENDER_VERSION%\include\makesdna;^
%cd%\External\blender-%BLENDER_VERSION%\include\makesrna;^
%cd%\External\blender-%BLENDER_VERSION%\include\python;
set BLENDER_PYTHON_INCLUDE_DIR=%cd%\External\python%BLENDER_PYTHON_VERSION%\include
set BLENDER_PYTHON_LIB_DIR=%cd%\External\python%BLENDER_PYTHON_VERSION%\lib64

set MAYA_VERSION=2019
set MAYA_LT=0
set MAYA_LIB_DIR=%cd%\External\Maya%MAYA_VERSION%\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya%MAYA_VERSION%\include

set MOTIONBUILDER_VERSION=2019
set MOTIONBUILDER_LIB_DIR=%cd%\External\OpenRealitySDK%MOTIONBUILDER_VERSION%\lib
set MOTIONBUILDER_INCLUDE_DIR=%cd%\External\OpenRealitySDK%MOTIONBUILDER_VERSION%\include

set MODO_QT_VERSION=4.8.5
set MODO_QT_DIR=%cd%\External\Qt\%MODO_QT_VERSION%
set MODO_SDK_DIR=LXSDK_525410
set MODO_INCLUDE_DIR=%cd%\External\%MODO_SDK_DIR%\include
set MODO_SOURCE_DIR=%cd%\External\%MODO_SDK_DIR%\common

set MQ4_VERSION=.70
set MQ4_SDK_DIR=%cd%\External\mqsdk470\mqsdk

MeshSync_All.sln
