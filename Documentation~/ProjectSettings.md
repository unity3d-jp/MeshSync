# Project Settings

1. [Server](#server)
1. [Scene Cache Player](#scene-cache-player)
1. [Editor Server](#editor-server)

## Server

![ProjectSettingsServer](images/ProjectSettingsServer.png)

Use the **Server** tab to set the default setting values for 
**MeshSyncServer** component.

|**Connection Settings**   |**Description** |
|:---                      |:---|
| **Server Port**          | The default server port.|
| **Allow public access**  | Allows public access to **MeshSyncServer**. <br/> By default, this setting is turned off, and only computers in local network     (127.0.0.1, 10.0.0.0/24, 192.168.0.0/16 or 172.16.0.0 to 172.31.255.255) can connect to **MeshSyncServer**. |

Please refer to [MeshSyncServer](MeshSyncServer.md) 
or [Common MeshSync Properties](CommonMeshSyncProperties.md)'s documentations 
for more details on the other settings.


## Scene Cache Player

![ProjectSettingsSceneCache](images/ProjectSettingsSceneCache.png)

Use the **Scene Cache Player** tab to set the default setting values for 
**SceneCachePlayer** component.  
Please refer to [SceneCache](SceneCache.md) 
or [Common MeshSync Properties](CommonMeshSyncProperties.md)'s documentations    
for more details on these settings.


## Editor Server

![](images/ProjectSettingsEditorServer.png)

Use the **Editor Server** tab to set the setting values for the MeshSync Editor Server 
that runs automatically by default for better integration with DCC tools.

|**Connection Settings**   |**Description** |
|:---                      |:---|
| **Server Port**          | The port of the currently running editor server. Changed will be applied immediately. |
| **Default Server Port**  | The default port for the editor server. Takes effect after restarting the editor. |
| **Start at Editor Launch**  | Runs the editor server automatically at launch if enabled. |



