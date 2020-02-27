Prerequisites:
- cmake: https://cmake.org/ 
- Poco: https://pocoproject.org
Download and build the static libs. Make install is not necessary

$ mkdir cmake-build
$ cd cmake-build
$ cmake .. -DBUILD_SHARED_LIBS=OFF && cmake --build . --config Release


- zstd
brew install zstd

Set environment variables:
export Poco_DIR=/Volumes/Data/SDK/poco


- Building on Mac:
cd Build
cmake -GXcode ..
xcodebuild -scheme mscore -configuration Release build
cp -r  ./Src/mscore/Release/mscore.bundle ../../Runtime/Plugins/x86_64


WINDOWS

Prerequisites
- cmake: https://cmake.org/ 
- Poco: https://pocoproject.org. Build  static libs
- zstd: https://github.com/facebook/zstd/releases
- Visual Studio 2017


Set environment variables:
Poco_DIR E:\SDK\poco


wget https://github.com/facebook/zstd/archive/v1.4.4.zip
unzip v1.4.4.zip
cd zstd-1.4.4\build\VS2010
devenv zstd.sln /upgrade
msbuild zstd.sln /p:Configuration=Debug /p:Platform=x64
msbuild zstd.sln /p:Configuration=Release /p:Platform=x64


$ cmake .. -DBUILD_SHARED_LIBS=OFF -G "Visual Studio 15 2017" -A x64 && cmake --build . --config Release

Steps:

-cd Build 
-cmake -G "Visual Studio 15 2017" -A x64 ..
- VsDevCmd_2017.bat
- msbuild MeshSyncProj.sln /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo



