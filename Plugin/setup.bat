@echo off

IF NOT EXIST "External\ispc.exe" (
    IF NOT EXIST "External\External.7z" (
        echo "downloading external libararies..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('https://github.com/i-saint/MeshSync/releases/download/0.0.1/External.7z', 'External/External.7z')"
    )
    cd External
    7z\7za.exe x -aos External.7z
    cd ..
)
IF NOT EXIST "External\mqsdk4" (
    IF NOT EXIST "External\mqsdk456.zip" (
        echo "downloading mqsdk456.zip ..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('http://www.metaseq.net/metaseq/mqsdk456.zip', 'External/mqsdk456.zip')"
        
    )
    cd External
    7z\7za.exe x -aos mqsdk456.zip
    mklink /J mqsdk4 mqsdk456\mqsdk
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
