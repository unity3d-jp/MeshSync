# Building zstd

Generate the binary by following the steps below, and copy it to the [External/zstd/lib](../../External/zstd/lib) folder depending on the platform.

## Windows

1. Install the following Visual Studio 2017 components:
   * Windows SDK 8.1
   * Windows Universal CRT SDK

1. Download [zstd v1.4.4](https://github.com/facebook/zstd/releases/download/v1.4.4/zstd-1.4.4.tar.gz) and extract to a folder.
1. Go to where zstd was extracted and execute the following:
```
$ cd build\VS2010
$ devenv zstd.sln /upgrade
$ msbuild zstd.sln /p:Configuration=Release /p:Platform=x64
```

## Mac 

1. Install [Homebrew](https://brew.sh/)
1. Install `zstd` via Homebrew and confirm that its version is 1.4.4.
    ``` 
    $ brew install zstd
    $ zstd --version
    ```  

## Linux

1. Download [zstd v1.4.4](https://github.com/facebook/zstd/releases/download/v1.4.4/zstd-1.4.4.tar.gz) and extract to a folder.
1. Go to where zstd was extracted and execute the following:
```
$ cd build/cmake
$ cmake . -DCMAKE_POSITION_INDEPENDENT_CODE=ON && cmake --build .
```


