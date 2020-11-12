# Preferences

Use the Preferences window to install 
[MeshSyncDCCPlugins](https://github.com/Unity-Technologies/MeshSyncDCCPlugins) 
in supported DCC Tools.  
When this window is opened for the first time, 
it automatically detects DCC tools which have been 
installed in their default locations automatically and 
display them inside the window.

![Preferences](../images/Preferences.png)

| Legend  | Use                                                                                       | 
| ------- | ----------------------------------------------------------------------------------------- | 
| A       | Display the name of the DCC tool, its path, and its MeshSyncDCCPlugins installation status|   
| B       | Launch the DCC tool                                                                       |   
| C       | Install MeshSyncDCCPlugins in the DCC tool                                                |  
| D       | Remove the DCC tool from the window                                                       |   
| E       | Detect and add DCC tools which have been installed in their default locations             |  
| F       | Add a DCC tool manually                                                                   | 

### Supported DCC Tools

1. Maya 2017 - 2020
2. 3ds Max 2017 - 2020
3. Blender 2.79, 2.80, 2.81, 2.82, 2.83, 2.90

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


