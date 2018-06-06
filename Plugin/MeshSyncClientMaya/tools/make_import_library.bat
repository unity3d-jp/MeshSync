echo off
call :MakeImportLibrary "C:\Program Files\Autodesk\MayaLT2018\bin\Foundation.dll" Foundation
call :MakeImportLibrary "C:\Program Files\Autodesk\MayaLT2018\bin\OpenMaya.dll" OpenMaya
call :MakeImportLibrary "C:\Program Files\Autodesk\MayaLT2018\bin\OpenMayaAnim.dll" OpenMayaAnim
exit /B 0

:MakeImportLibrary
    dumpbin /exports "%~1" > %~2.txt
    echo LIBRARY %~3 > %~2.def
    echo EXPORTS >> %~2.def
    for /f "skip=19 tokens=4" %%l in (%~2.txt) do echo %%l >> %~2.def
    lib /def:%~2.def /out:%~2.lib /machine:x64
    del %~2.txt
