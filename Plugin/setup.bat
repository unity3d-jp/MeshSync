@echo off

IF NOT EXIST "External\ispc.exe" (
    IF NOT EXIST "External\External.7z" (
        echo "downloading external libararies..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('https://github.com/i-saint/MeshSync/releases/download/0.0.0/External.7z', 'External/External.7z')"
    )
    cd External
    7z\7za.exe x -aos *.7z
    cd ..
)
IF NOT EXIST "External\mqsdk" (
    IF NOT EXIST "External\mqsdk456.zip" (
        echo "downloading mqsdk ..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('http://www.metaseq.net/metaseq/mqsdk456.zip', 'External/mqsdk456.zip')"
        
    )
    cd External
    7z\7za.exe x -aos *.zip
    mklink /J mqsdk mqsdk456\mqsdk
    cd ..
)
