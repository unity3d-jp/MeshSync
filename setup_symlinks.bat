@echo off

for /D %%s in (MeshSync*) do (
echo Setting up symlinks in %%s
cd %%s\Packages\com.unity.meshsync

del /f /q Editor
mklink /D Editor "..\..\..\Editor"

del /f /q Runtime
mklink /D Runtime "..\..\..\Runtime"

del /f /q Tests
mklink /D Tests "..\..\..\Tests"

del /f /q Documentation~
mklink /D Documentation~ "..\..\..\Documentation~"

cd ../../..
)

