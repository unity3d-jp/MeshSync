call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"

msbuild MeshSyncServer.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
