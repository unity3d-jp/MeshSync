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
    IF NOT EXIST "External\mqsdk311.zip" (
        echo "downloading mqsdk311.zip ..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('http://www.metaseq.net/metaseq/mqsdk311.zip', 'External/mqsdk311.zip')"
        
    )
    cd External
    7z\7za.exe x -omqsdk311 -aos mqsdk311.zip
    mklink /J mqsdk3 mqsdk311\mqsdk
    cd ..
)
