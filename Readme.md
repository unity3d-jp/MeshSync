![demo](Documentation~/images/Demo.gif)
# Other Languages
- [日本語](Readme_JP.md)


# MeshSync

Working together with [MeshSyncDCCPlugins](https://github.com/Unity-Technologies/MeshSyncDCCPlugins), MeshSync is a package for synchronizing meshes/models editing in DCC tools into Unity in real time.  
This allows devs to immediately see how things will look in-game while modelling.  

Installation is done via the [Package Manager](https://docs.unity3d.com/Manual/Packages.html).  
Please ensure that *Show preview packages* is checked to search for MeshSync.
![Menu](Documentation~/images/PackageManager.png)


## Supported Platforms

- Windows 64 bit
- Mac
- Linux

# Basic usage

From the **GameObject** menu, choose **MeshSync > Create Server** to create a server object.  
This object has [MeshSyncServer](Documentation~/en/MeshSyncServer.md) component that handles the sync process.

![Menu](Documentation~/images/MenuCreateServer.png)

# Settings

Various MeshSync related settings, including DCC Tools integration, can be configured on the 
[Project Settings](Documentation~/en/ProjectSettings.md) window.

![Settings](Documentation~/images/ProjectSettings.png)

# Advanced Features
- [SceneCache](Documentation~/en/SceneCache.md)


# Plugins
- [Building](Plugin~/Docs/en/BuildPlugins.md)

# License
- [License](LICENSE.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Third Party Notices](Third%20Party%20Notices.md)
- [Contributing](CONTRIBUTING.md)

#  Related Tools
- [NormalPainter](https://github.com/unity3d-jp/NormalPainter): Tool for editing vectors in Unity
- [BlendShapeBuilder](https://github.com/unity3d-jp/BlendShapeBuilder): Tool for building BlendShapes in Unity

