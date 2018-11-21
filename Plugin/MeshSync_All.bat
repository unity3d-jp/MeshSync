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
set MAYA_LIB_DIR=%cd%\External\Maya%MAYA_VERSION%\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya%MAYA_VERSION%\include

set MOTIONBUILDER_VERSION=2018
set MOTIONBUILDER_LIB_DIR=%cd%\External\OpenRealitySDK%MOTIONBUILDER_VERSION%\lib
set MOTIONBUILDER_INCLUDE_DIR=%cd%\External\OpenRealitySDK%MOTIONBUILDER_VERSION%\include

set XISMO_VERSION=190
set XISMO_QT_VERSION=5.6.2
set XISMO_QT_DIR=%cd%\External\Qt\%XISMO_QT_VERSION%\msvc2015_64

set VRED_VERSION=2019
set VRED_QT_VERSION=5.9.3
set VRED_QT_DIR=%cd%\External\Qt\%VRED_QT_VERSION%\msvc2017_64

MeshSync_All.sln
