@echo off

cd External

IF NOT EXIST "7za.exe" (
    echo "downloading 7za.exe..."
    powershell.exe -NoProfile -InputFormat None -ExecutionPolicy Bypass -Command "[System.Net.ServicePointManager]::SecurityProtocol=[System.Net.SecurityProtocolType]::Tls12; wget https://github.com/unity3d-jp/MeshSync/releases/download/20171228/7za.exe -OutFile 7za.exe"
)

echo "downloading external libararies..."
powershell.exe -NoProfile -InputFormat None -ExecutionPolicy Bypass -Command "[System.Net.ServicePointManager]::SecurityProtocol=[System.Net.SecurityProtocolType]::Tls12; wget https://github.com/unity3d-jp/MeshSync/releases/download/20200123/External.7z -OutFile External.7z"
7za.exe x -aos External.7z

echo "downloading mqsdk470.zip ..."
powershell.exe -Command "wget http://www.metaseq.net/metaseq/mqsdk470.zip -OutFile mqsdk470.zip"
7za.exe x -aos -o"mqsdk470" mqsdk470.zip

echo "downloading mqsdk464.zip ..."
powershell.exe -Command "wget http://www.metaseq.net/metaseq/mqsdk464.zip -OutFile mqsdk464.zip"
7za.exe x -aos mqsdk464.zip

echo "downloading mqsdk249c.zip ..."
powershell.exe -Command "wget http://www.metaseq.net/metaseq/mqsdk249c.zip -OutFile mqsdk249c.zip"
7za.exe x -aos -o"mqsdk249c" mqsdk249c.zip
mklink /J mqsdk3 mqsdk249c\mqsdk

cd ..
