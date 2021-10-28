# Preferences

Use the Preferences window to install 
[MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
in [supported DCC Tools](#supported-dcc-tools).  
When this window is opened for the first time, 
it automatically detects DCC tools which have been 
installed in their default locations, and 
displays them inside the window.

![Preferences](images/Preferences.png)

| Legend  | Use                                                                                       | 
| :---    | :---                                                                                      | 
| A       | Display the name of the DCC tool, its path, and its MeshSyncDCCPlugins installation status|   
| B       | Launch the DCC tool                                                                       |   
| C       | Install MeshSyncDCCPlugins in the DCC tool                                                |  
| D       | Remove the DCC tool from the window                                                       |   
| E       | Detect and add DCC tools which have been installed in their default locations             |  
| F       | Add a DCC tool manually                                                                   | 

## Supported DCC Tools

1. Maya 2018 - 2022
2. 3ds Max 2018 - 2021
3. Blender 2.83, 2.90, 2.91, 2.92, 2.93

## Notes

1. Please close all instances of a DCC tool before installing the plugin for that DCC tool.
2. For some DCC tools, MeshSync plugin installation will launch the DCC tool automatically 
   to finalize the installation process.

## MeshSyncDCCPlugins installation info

The installation info is stored in the following location.

| Operating System  | Path                                              | 
| :---              | :---                                              | 
| Linux             | ~/.config/unity3d/Unity/MeshSync                  |   
| macOS             | ~/Library/Application Support/Unity/MeshSync      |   
| Windows           | C:\Users\username\AppData\Local\Unity\MeshSync    |  


