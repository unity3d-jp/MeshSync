![demo](Documentation~/images/Demo.gif)

# Latest official docs
- [English](https://docs.unity3d.com/Packages/com.unity.meshsync@latest)
- [日本語](https://docs.unity3d.com/ja/Packages/com.unity.meshsync@latest)


# MeshSync

[![](https://badge-proxy.cds.internal.unity3d.com/f4d1069b-1233-4324-ae75-2fac980576a4)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync/build-info?branch=dev&testWorkflow=package-isolation)
[![](https://badge-proxy.cds.internal.unity3d.com/6cfcda56-5e8d-4612-abfa-6de23df068fb)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync/dependencies-info?branch=dev&testWorkflow=updated-dependencies)
[![](https://badge-proxy.cds.internal.unity3d.com/45cf24da-7561-4983-9e11-fc920996015c)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync/dependants-info)
[![](https://badge-proxy.cds.internal.unity3d.com/0998ee7c-b3f2-4ef9-97cd-628296f29c4a)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync/warnings-info?branch=dev)

![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/9cb90abe-572c-440c-b7f9-f212c5573261)
![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/4661afc4-7953-410d-a4fa-9668ed7da2b9)

Working together with [MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest), 
MeshSync is a package for synchronizing meshes/models editing in DCC tools into Unity in real time. 
This allows devs to immediately see how things will look in-game while modelling.  

## Supported Platforms

- Windows 64 bit
- Mac
- Linux

# Basic usage

From the **GameObject** menu, choose **MeshSync > Create Server** to create a server object.  
This object has [MeshSyncServer](Documentation~/MeshSyncServer.md) component that handles the sync process.

![Menu](Documentation~/images/MenuCreateServer.png)

Please refer to the [installation](Documentation~/Installation.md) page to install MeshSync package.

# Features

* Components:
    * [MeshSyncServer](Documentation~/MeshSyncServer.md): to sync meshes/models editing in DCC tools into Unity in real time.
    * [SceneCache](Documentation~/SceneCache.md): to playback all frames of an *.sc* file exported using [MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
      * [SceneCache in Timeline](Documentation~/SceneCacheInTimeline.md)
* [Project Settings](Documentation~/ProjectSettings.md): to configure default settings for MeshSync components. 
* [Preferences](Documentation~/Preferences.md): to configure DCC Tools integration.

<img align="center" src="Documentation~/images/Preferences.png" height=360>


# Internal Plugins
- [Building](Plugin~/Docs/en/BuildPlugins.md)

# License
- [License](LICENSE.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Third Party Notices](Third%20Party%20Notices.md)
- [Contributing](CONTRIBUTING.md)

#  Related Tools
- [NormalPainter](https://github.com/unity3d-jp/NormalPainter): Tool for editing vectors in Unity
- [BlendShapeBuilder](https://github.com/unity3d-jp/BlendShapeBuilder): Tool for building BlendShapes in Unity

