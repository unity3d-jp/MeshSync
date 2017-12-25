@echo off

IF NOT EXIST "External\ispc.exe" (
    IF NOT EXIST "External\External.7z" (
        echo "downloading external libararies..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('https://github.com/unity3d-jp/MeshSync/releases/download/0.0.2/External.7z', 'External/External.7z')"
    )
    cd External
    7z\7za.exe x -aos External.7z
    cd ..
)
IF NOT EXIST "External\mqsdk4" (
    IF NOT EXIST "External\mqsdk464.zip" (
        echo "downloading mqsdk464.zip ..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('http://www.metaseq.net/metaseq/mqsdk464.zip', 'External/mqsdk464.zip')"
    )
    cd External
    7z\7za.exe x -aos mqsdk464.zip
    mklink /J mqsdk4 mqsdk464\mqsdk
    cd ..
)
IF NOT EXIST "External\mqsdk3" (
    IF NOT EXIST "External\mqsdk249c.zip" (
        echo "downloading mqsdk249c.zip ..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('http://www.metaseq.net/metaseq/mqsdk249c.zip', 'External/mqsdk249c.zip')"
    )
    cd External
    7z\7za.exe x -omqsdk249c -aos mqsdk249c.zip
    mklink /J mqsdk3 mqsdk249c\mqsdk
    cd ..
)
