call "%VS2017INSTALLDIR%\Common7\Tools\VsDevCmd.bat"

msbuild PyMeshSync.vcxproj /t:Build /p:Configuration=Master /p:Platform=x64 /m /nologo
