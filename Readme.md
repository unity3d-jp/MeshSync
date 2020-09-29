![demo](Documentation~/images/Demo.gif)
# Other Languages
- [日本語](Readme_JP.md)


# MeshSync

Working together with [MeshSyncDCCPlugins](https://github.com/Unity-Technologies/MeshSyncDCCPlugins), MeshSync is a package for synchronizing meshes/models editing in DCC tools into Unity in real time. 
This allows devs to immediately see how things will look in-game while modelling.  

MeshSync is currently a preview package and the steps to install it 
differ based on the version of Unity.

* Unity 2019.x  
  ![PackageManager2019](Documentation~/images/PackageManager2019.png)
  1. Open [Package Manager](https://docs.unity3d.com/Manual/upm-ui.html) 
  2. Ensure that **Show preview packages** is checked. 
  3. Search for *MeshSync*.
  
* Unity 2020.1  
  ![PackageManager2020](Documentation~/images/PackageManager2020.1.png)
  1. Open [Package Manager](https://docs.unity3d.com/Manual/upm-ui.html) 
  2. Click the **+** button, and choose **Add package from git URL** 
  3. Type in `com.unity.meshsync@` followed by the version.  
     For example: `com.unity.meshsync@0.2.5-preview`


## Supported Platforms

- Windows 64 bit
- Mac
- Linux

# Basic usage

From the **GameObject** menu, choose **MeshSync > Create Server** to create a server object.  
This object has [MeshSyncServer](Documentation~/en/MeshSyncServer.md) component that handles the sync process.

![Menu](Documentation~/images/MenuCreateServer.png)

# Settings

Default settings for MeshSync components can be configured on the 
[Project Settings](Documentation~/en/ProjectSettings.md) window.

![Server Settings](Documentation~/images/ProjectSettingsServer.png)

Similarly, DCC Tools integration can be configured on the 
[Preferences](Documentation~/en/Preferences.md) window

![Server Settings](Documentation~/images/Preferences.png)


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

