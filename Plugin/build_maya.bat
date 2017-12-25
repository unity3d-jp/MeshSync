call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"

set MAYA_VERSION=2016
set MAYA_LIB_DIR=%cd%\External\Maya2016\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya2016\include
msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "UnityMeshSync for Maya\Maya2016"
copy _out\MeshSyncClientMaya2016_x64_Master\MeshSyncClientMaya.mll "UnityMeshSync for Maya\Maya2016"
copy MeshSyncClientMaya\MEL\*.mel "UnityMeshSync for Maya\Maya2016"

set MAYA_VERSION=2016.5
set MAYA_LIB_DIR=%cd%\External\Maya2016.5\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya2016.5\include
msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "UnityMeshSync for Maya\Maya2016.5"
copy _out\MeshSyncClientMaya2016.5_x64_Master\MeshSyncClientMaya.mll "UnityMeshSync for Maya\Maya2016.5"
copy MeshSyncClientMaya\MEL\*.mel "UnityMeshSync for Maya\Maya2016.5"

set MAYA_VERSION=2017
set MAYA_LIB_DIR=%cd%\External\Maya2017\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya2017\include
msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "UnityMeshSync for Maya\Maya2017"
copy _out\MeshSyncClientMaya2017_x64_Master\MeshSyncClientMaya.mll "UnityMeshSync for Maya\Maya2017"
copy MeshSyncClientMaya\MEL\*.mel "UnityMeshSync for Maya\Maya2017"

set MAYA_VERSION=2018
set MAYA_LIB_DIR=%cd%\External\Maya2018\lib
set MAYA_INCLUDE_DIR=%cd%\External\Maya2018\include
msbuild MeshSyncClientMaya.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
mkdir "UnityMeshSync for Maya\Maya2018"
copy _out\MeshSyncClientMaya2018_x64_Master\MeshSyncClientMaya.mll "UnityMeshSync for Maya\Maya2018"
copy MeshSyncClientMaya\MEL\*.mel "UnityMeshSync for Maya\Maya2018"

del "UnityMeshSync for Maya.zip"
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('UnityMeshSync for Maya', 'UnityMeshSync for Maya.zip'); }"
