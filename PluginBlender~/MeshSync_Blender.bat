set BLENDER_VERSION=2.79
set BLENDER_PYTHON_VERSION=35
set PLUGIN_DIR=%cd%\..\Plugin~
set BLENDER_INCLUDE_DIRS=^
%PLUGIN_DIR%\External\blender-%BLENDER_VERSION%\include\blenkernel;^
%PLUGIN_DIR%\External\blender-%BLENDER_VERSION%\include\blenlib;^
%PLUGIN_DIR%\External\blender-%BLENDER_VERSION%\include\bmesh;^
%PLUGIN_DIR%\External\blender-%BLENDER_VERSION%\include\makesdna;^
%PLUGIN_DIR%\External\blender-%BLENDER_VERSION%\include\makesrna;^
%PLUGIN_DIR%\External\blender-%BLENDER_VERSION%\include\python;
set BLENDER_PYTHON_INCLUDE_DIR=%PLUGIN_DIR%\External\python%BLENDER_PYTHON_VERSION%\include
set BLENDER_PYTHON_LIB_DIR=%PLUGIN_DIR%\External\python%BLENDER_PYTHON_VERSION%\lib64
MeshSync_Blender.sln