echo off

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" /latest /property installationPath`) do (
  set VSDIR=%%i
)
call "%VSDIR%\Common7\Tools\VsDevCmd.bat"
cd %~dp0

call :MakeImportLibrary "C:\Program Files\Autodesk\MayaLT2019\bin\Foundation.dll" Foundation
call :MakeImportLibrary "C:\Program Files\Autodesk\MayaLT2019\bin\OpenMaya.dll" OpenMaya
call :MakeImportLibrary "C:\Program Files\Autodesk\MayaLT2019\bin\OpenMayaAnim.dll" OpenMayaAnim
pause
exit /B 0

:MakeImportLibrary
    dumpbin /exports "%~1" > %~2.txt
    echo LIBRARY %~3 > %~2.def
    echo EXPORTS >> %~2.def
    for /f "skip=19 tokens=4" %%l in (%~2.txt) do echo %%l >> %~2.def
    lib /def:%~2.def /out:%~2.lib /machine:x64
    undname %~2.txt > %~2.und.txt
