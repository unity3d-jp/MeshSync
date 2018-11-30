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
MeshSync_Blender.sln