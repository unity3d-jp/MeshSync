![demo](images/Demo.gif)
# MeshSync

Working together with [MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest), 
MeshSync is a package for synchronizing meshes/models editing in DCC tools into Unity in real time.
This allows devs to immediately see how things will look in-game while modelling.  

## Supported Platforms

- Windows 64 bit
- Mac
- Linux

# Basic usage

From the **GameObject** menu, choose **MeshSync > Create Server** to create a server object.  
This object has [MeshSyncServer](MeshSyncServer.md) component that handles the sync process.

![Menu](images/MenuCreateServer.png)

# Settings

Default settings for MeshSync components can be configured on the 
[Project Settings](ProjectSettings.md) window.

![Server Settings](images/ProjectSettingsServer.png)

Similarly, DCC Tools integration can be configured on the 
[Preferences](Preferences.md) window

![Server Settings](images/Preferences.png)


# Advanced Features
- [SceneCache](SceneCache.md)
  - [SceneCache in Timeline](SceneCacheInTimeline.md)









