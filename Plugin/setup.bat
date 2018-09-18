@echo off

echo "downloading external libararies..."
powershell.exe -Command "[System.Net.ServicePointManager]::SecurityProtocol=[System.Net.SecurityProtocolType]::Tls12; wget https://github.com/unity3d-jp/MeshSync/releases/download/20171228/External.7z -OutFile External/External.7z"
cd External
7z\7za.exe x -aos External.7z
cd ..

echo "downloading mqsdk464.zip ..."
powershell.exe -Command "wget http://www.metaseq.net/metaseq/mqsdk464.zip -OutFile External/mqsdk464.zip"
cd External
7z\7za.exe x -aos mqsdk464.zip
mklink /J mqsdk4 mqsdk464\mqsdk
cd ..

echo "downloading mqsdk249c.zip ..."
powershell.exe -Command "wget http://www.metaseq.net/metaseq/mqsdk249c.zip -OutFile External/mqsdk249c.zip"
cd External
7z\7za.exe x -omqsdk249c -aos mqsdk249c.zip
mklink /J mqsdk3 mqsdk249c\mqsdk
cd ..
