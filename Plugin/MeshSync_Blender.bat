set BLENDER_VERSION=2.79
set PYTHON_VERSION=35
set BLENDER_INCLUDE_DIRS=^
%cd%\External\blender-%BLENDER_VERSION%\include\blenkernel;^
%cd%\External\blender-%BLENDER_VERSION%\include\blenlib;^
%cd%\External\blender-%BLENDER_VERSION%\include\bmesh;^
%cd%\External\blender-%BLENDER_VERSION%\include\makesdna;^
%cd%\External\blender-%BLENDER_VERSION%\include\makesrna;^
%cd%\External\blender-%BLENDER_VERSION%\include\python;
set PYTHON_INCLUDE_DIR=%cd%\External\python%PYTHON_VERSION%\include
set PYTHON_LIB_DIR=%cd%\External\python%PYTHON_VERSION%\lib64
MeshSync_Blender.sln