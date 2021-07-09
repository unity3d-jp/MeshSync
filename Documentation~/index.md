![demo](images/Demo.gif)
# MeshSync

Working together with [MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest), 
MeshSync is a package for synchronizing meshes/models editing in DCC tools into Unity in real time.
This allows devs to immediately see how things will look in-game while modelling.  

## Supported Platforms

- Windows 64 bit
- Mac
- Linux

Please refer to the [installation](Installation.md) page to install MeshSync package.

# Basic usage

From the **GameObject** menu, choose **MeshSync > Create Server** to create a server object.  
This object has [MeshSyncServer](MeshSyncServer.md) component that handles the sync process.

![Menu](images/MenuCreateServer.png)

# Features

* Components:
    * [MeshSyncServer](MeshSyncServer.md): to sync meshes/models editing in DCC tools into Unity in real time.
    * [SceneCache](SceneCache.md): to playback all frames of an *.sc* file exported using [MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
      * [SceneCache in Timeline](SceneCacheInTimeline.md)
* [Project Settings](ProjectSettings.md): to configure default settings for MeshSync components. 
* [Preferences](Preferences.md): to configure DCC Tools integration.

<p align="center">
    <img src="images/Preferences.png" height=360>
</p>

