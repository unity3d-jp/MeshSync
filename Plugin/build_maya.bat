call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"

set MAYA_VERSION=2016
set MAYA_LIB_DIR=C:\Program Files\Autodesk\Maya2016\lib
set MAYA_INCLUDE_DIR=C:\SSD\Projects\MeshSync\Plugin\External\Maya2016_DEVKIT_Windows\devkitBase\include
msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "UnityMeshSync for Maya\Maya2016"
copy _out\MeshSyncClientMaya2016_x64_Master\MeshSyncClientMaya.mll "UnityMeshSync for Maya\Maya2016"

set MAYA_VERSION=2017
set MAYA_LIB_DIR=C:\Program Files\Autodesk\Maya2017\lib
set MAYA_INCLUDE_DIR=C:\SSD\Projects\MeshSync\Plugin\External\Maya2017_DEVKIT_Windows\devkitBase\include
msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "UnityMeshSync for Maya\Maya2017"
copy _out\MeshSyncClientMaya2017_x64_Master\MeshSyncClientMaya.mll "UnityMeshSync for Maya\Maya2017"
