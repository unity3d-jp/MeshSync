set MAX_VERSION=2019
set MAX_LIB_DIR=%cd%\External\3ds Max %MAX_VERSION% SDK\maxsdk\lib\x64\Release
set MAX_INCLUDE_DIR=%cd%\External\3ds Max %MAX_VERSION% SDK\maxsdk\include

set BLENDER_VERSION=2.79
set BLENDER_PYTHON_VERSION=35
set BLENDER_INCLUDE_DIRS=^
%cd%\External\blender-%BLENDER_VERSION%\include\blenkernel;^
%cd%\External\blender-%BLENDER_VERSION%\include\blenlib;^
%cd%\External\blender-%BLENDER_VERSION%\include\bmesh;^
%cd%\External\blender-%BLENDER_VERSION%\include\makesdna;^
%cd%\External\blender-%BLENDER_VERSION%\include\makesrna;^
%cd%\External\blender-%BLENDER_VERSION%\include\python;
set BLENDER_PYTHON_INCLUDE_DIR=%cd%\External\python%BLENDER_PYTHON_VERSION%\include
set BLENDER_PYTHON_LIB_DIR=%cd%\External\python%BLENDER_PYTHON_VERSION%\lib64

set MAYA_VERSION=2018
set MAYA_LT=0
set MAYA_LIB_DIR=%cd%\External\Maya2018\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya2018\include

set MOTIONBUILDER_VERSION=2018
set MOTIONBUILDER_LIB_DIR=%cd%\External\OpenRealitySDK2018\lib
set MOTIONBUILDER_INCLUDE_DIR=%cd%\External\OpenRealitySDK2018\include

MeshSync_All.sln
