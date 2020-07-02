# Project Settings

1. [General Settings](#general-settings)
1. [DCC Tools](#dcc-tools)

## General Settings

![ProjectSettingsGeneral](../images/ProjectSettingsGeneral.png)

Use the general settings tab to set the default setting values for 
1. The server:
   * **Server Port**  
     The default server port.
   * **Allow public access**  
     Allows public access to MeshSync. By default, this setting is turned off, 
     and only computers in local network 
     (127.0.0.1, 10.0.0.0/24, 192.168.0.0/16 or 172.16.0.0 to 172.31.255.255)
     can connect to MeshSync.

2. MeshSync objects
   1. [MeshSyncServer](MeshSyncServer.md)
   2. [SceneCache](SceneCache.md)



## DCC Tools

Use the DCC Tools tab to install 
[MeshSyncDCCPlugins](https://github.com/Unity-Technologies/MeshSyncDCCPlugins) 
in supported DCC Tools.  
When this tab is opened for the first time, 
it automatically detects DCC tools which have been 
installed in their default locations automatically and 
display them inside the tab.

![ProjectSettingsDCCTools](../images/ProjectSettingsDCCTools.png)

| Legend  | Use                                                                                       | 
| ------- | ----------------------------------------------------------------------------------------- | 
| A       | Display the name of the DCC tool, its path, and its MeshSyncDCCPlugins installation status|   
| B       | Remove the DCC tool from the tab                                                          |   
| C       | Install MeshSyncDCCPlugins in the DCC tool                                                |  
| D       | Detect and add DCC tools which have been installed in their default locations             |  
| E       | Add a DCC tool manually                                                                   | 

### Supported DCC Tools

1. Maya 2017 - 2020
2. 3ds Max 2017 - 2020
3. Blender 2.79, 2.80, 2.81, 2.82, 2.83

### Notes

1. Please close all instances of a DCC tool before installing MeshSyncDCCPlugins.
2. For some DCC tools, MeshSync plugin installation will launch the DCC tool automatically 
   to finalize the MeshSyncDCCPlugins installation process.

### MeshSyncDCCPlugins installation info

The installation info is stored in the following location.

| Operating System  | Path                                              | 
| ----------------- | --------------------------------------------------| 
| Linux             | ~/.config/unity3d/Unity/MeshSync                  |   
| macOS             | ~/Library/Application Support/Unity/MeshSync      |   
| Windows           | C:\Users\username\AppData\Local\Unity\MeshSync    |  


