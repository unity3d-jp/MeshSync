call buildtools.bat

set MAYA_VERSION=2016.5
set MAYA_LIB_DIR=%cd%\External\Maya2016.5\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya2016.5\include
set DIST_DIR="dist\UnityMeshSync_maya2016_Windows\UnityMeshSync"
msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "%DIST_DIR%\plug-ins"
mkdir "%DIST_DIR%\scripts"
copy _out\x64_Master\MeshSyncClientMaya2016.5\MeshSyncClientMaya.mll "%DIST_DIR%\plug-ins"
copy MeshSyncClientMaya\MEL\*.mel "%DIST_DIR%%\scripts"

set MAYA_VERSION=2017
set MAYA_LIB_DIR=%cd%\External\Maya2017\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya2017\include
set DIST_DIR="dist\UnityMeshSync_maya2017_Windows\UnityMeshSync"
msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "%DIST_DIR%\plug-ins"
mkdir "%DIST_DIR%\scripts"
copy _out\x64_Master\MeshSyncClientMaya2017\MeshSyncClientMaya.mll "%DIST_DIR%\plug-ins"
copy MeshSyncClientMaya\MEL\*.mel "%DIST_DIR%\scripts"

set MAYA_VERSION=2018
set MAYA_LIB_DIR=%cd%\External\Maya2018\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya2018\include
set DIST_DIR="dist\UnityMeshSync_maya2018_Windows\UnityMeshSync"
msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "%DIST_DIR%\plug-ins"
mkdir "%DIST_DIR%\scripts"
copy _out\x64_Master\MeshSyncClientMaya2018\MeshSyncClientMaya.mll "%DIST_DIR%\plug-ins"
copy MeshSyncClientMaya\MEL\*.mel "%DIST_DIR%%\scripts"
