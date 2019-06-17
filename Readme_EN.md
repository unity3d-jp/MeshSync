![demo](https://user-images.githubusercontent.com/1488611/39971828-98afa1d8-573d-11e8-9a6f-86263bee8949.gif)
# MeshSync

MeschSync reflects changes to models made in DCC tools in real time in Unity. This allows devs to immediately see how things will look in-game while modelling.  
MeshSync works as a plugin for Unity and DCC tools, and currently supports: [Maya](https://www.autodesk.eu/products/maya/overview), [Maya LT](https://www.autodesk.eu/products/maya-lt/overview), [3ds Max](https://www.autodesk.com/products/3ds-max/overview), [Blender](https://blenderartists.org/), [Metaseq](http://www.metaseq.net/), and [xismo](http://mqdl.jpn.org/).


## Guides
1. [Maya](#maya)
2. [Maya LT](#maya-lt)
3. [3ds Max](#3ds-max)
4. [Motion Builder](#motion-builder)
5. [Blender](#blender)
6. [Modo](#modo)
7. [Metaseq](#Metaseq)
8. [VRED](#vred)
9. [xismo](#xismo)
10. [Unity](#unity)

<img align="right" src="https://user-images.githubusercontent.com/1488611/39971860-7f6d1330-573e-11e8-9a1e-9d95709cbd50.png" height=400>

### Maya
Confirmed functionality with Maya 2015, 2016, 2016.5, 2017, 2018, 2019 + Windows, Mac, and Linux (CentOS 7).
- Installation:
  - Download UnityMeshSync_Maya_*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases).
  - Windows: If %MAYA_APP_DIR% is already setup, copy the modules directory there, if not copy it to %USERPROFILE%\Documents\maya (← copy paste into the Explorer address bar).
    - For versions prior to 2016, copy to the version name directory (%MAYA_APP_DIR%\2016 etc.)
  - Mac: Copy the UnityMeshSync directory and .mod file to /Users/Shared/Autodesk/modules/maya.
  - Linux: Copy the modules directory to ~/maya/(Maya version).
- Start Maya, then go to Windows -> Settings/Preferences -> Plug-in Manager, and activate the plugin by checking Loaded under MeshSyncClient.
- Now that the UnityMeshSync shelf has been added, click on the gear icon to open the settings menu. 
- While "Auto Sync" is checked, any edits will automatically be reflected in Unity. When Auyo Sync is deactivated, click the  "Manual Sync" button to sync changes.
- Clicking Sync under Animations causes the timer to advance from the first frame to the final frame while baking the animation and sending it to Unity. 

&nbsp;  

- The other buttons correspond to their respective manual sync and animation sync functions. 
- Polygon mesh, camera, and light sync are supported. 
- Polygon mesh will carry skinning/bones (SkinCluster) and BlendShapes over to Unity as is.
  - MeshSync will attempt to apply any additional deformers, but if there is a SkinCluster before or after them they may not apply correctly. 
  - Check "Bake Deformers" to sync the results of applying all deformers. This will mostly sync the Mesh on both the Maya and Unity sides, but this will result in loss of Skinning and BlendShape information.
- Checing "Double Sided" will cause the Mesh to become double-sided on the Unity side
- Be advised that the negative scale is only supported for certain elements. 
  - If XYZ all have negative values, the Mesh will sync properly, however if only one axis has a negative value Unity will treat the Mesh as though every axis has a negative value.
- Non-polygon shape data such as NURBS is not supported.
- Instancing is supported, but instancing for skinned meshes is currently not supported (on the Unity side they all end up in the same position as the original instance). 
- Commands are also registered to MEL, and all features can be accessed through MEL. See [the source code] (https://github.com/unity3d-jp/MeshSync/blob/master/.MeshSync/Plugin/MeshSyncClientMaya/msmayaCommands.cpp) for details.


### Maya LT
Currently, only Windows is supported, and the tool is confirmed to work on Maya LT 2019 + Windows. Maya LT does not natively support outside plugins, so be aware that this may lead to problems. Even small version changes to Maya LT may lead to loss of compatibility.   
This is a separate package, but the process for installation and use is the same as [Non-LT Maya](#maya).


### 3ds Max
Confirmed functionality with 3ds Max 2016, 2017, 2018, 2019, 2020 + Windows.
- Installation: 
  - Donload UnityMeshSync_3dsMax_Windows.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases)
  - Copy MeshSyncClient3dsMax.dlu into the directory for plugin paths.
    - Plugin paths can be added in Max by going to Add under Customize -> Configure User and System Paths -> 3rd Party Plug-Ins
    - The default path (C:\Program Files\Autodesk\3ds Max 2019\Plugins) should be fine, but using a separate path is recommended
- After installing, "UnityMeshSync" will be added to the main menu bar, and the settings window can be opened by clicking "Window". 
  - If you change the menu bar, the "UnityMeshSync" category will be added under Action, where MeshSync features can also be accessed
- While "Auto Sync" is checked, changes to the Mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to sync changes manually.  
- Clicking Sync under Animations will cause the timer to advance from the first frame to the final frame while baking the animation before sending it to Unity.

&nbsp;  

- Polygon mesh, camera, and light sync are supported. 
- Most modifiers are supported, but are unsupported in some cases. Use the following rules.  
  - When there is no Morph or Skin, all modifiers will be applied during sync. 
  - If there is a Morph or Skin, all modifiers before them will be applied during sync.  
    - If there are multiple Morphs / Skins, the one at the bottom will be chosen as the base.
  - Morphs and Skins will sync on the Unity side as Blendshapes / Skins.
    - Unity applies them in order of Blendshape -> Skin, so if the order is reversed in Max, unintentional results may occur.
  - If "Bake Deformers" is checked, the results of applying all deformers will be sent to Unity. This will keep the content of the Mesh mostly consistent between Max and Unity, but will also result in the loss of Skinning and Blendshape information.
- Checking "Double Sided" will make the Mesh double-sided in Unity.
- Be advised that the negative scale is only supported for certain elements.
  - If XYZ all have negative values, the Mesh will sync properly, however if only one axis has a negative value Unity will treat the Mesh as though every axis has a negative value.
- Commands have also been added to the Max script, so all features can be accessed via the Max script.See [the source code] (https://github.com/unity3d-jp/MeshSync/blob/master/.MeshSync/Plugin/MeshSyncClient3dsMax/msmaxEntryPoint.cpp) for details. 


<img align="right" src="https://user-images.githubusercontent.com/1488611/45682175-8a919100-bb7a-11e8-96a1-efe2e28146c3.png" height=200>

### Motion Builder
Confirmed functionality with Motion Builder 2015, 2016, 2017, 2018, 2019 + Windows, Linux (CentOS 7) 
- Installation:
  - Download UnityMeshSync_MotionBuilder_*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases)
  - Copy MeshSyncClientMotionBuilder.dll to the directory registered as a plugin path 
    - Plugin paths can be added in Motion Builder under Settings -> Preferences -> SDK menu
- After installation an object called UnityMeshSync will be added to the Asset Browser under Templates -> Devices, so add it to the scene  
- The various settings and features can be accessed in the Navigator by selecting Devices -> UnityMeshSync 
- While "Auto Sync" is checked, any changes to the Mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to manually reflect changes  
- Clicking Sync under Animations causes the timer to advance from the first frame to the final frame while baking the animation before sending it to Unity.  

&nbsp;  

- Polygon mesh, camera, and lighting sync are supported.
- The Polygon mesh's skinning/bone and BlendShapes will be carried over to Unity unchanged. 
- Checking "Double Sided" causes the Mesh to become double-sided in Unity. 
- Be advised that the negative scale is only supported for certain elements.
  - If XYZ all have negative values, the Mesh will sync properly, however if only one axis has a negative value Unity will treat the Mesh as though every axis has a negative value.
- Non-polygon shape data such as NURBS is not supported. 


<img align="right" src="https://user-images.githubusercontent.com/1488611/49272332-79d39480-f4b4-11e8-8ca3-0ce0bc90a965.png" height=400>

### Blender
Functionality confirmed with Blender 2.79(a,b), 2.80 beta (2019-4-23) + Windows, Mac, Linux (CentOS 7). Be aware that depending on the implementation, **there is a high possibility that upgrading the Blender version will lead to a loss of compatibility**. Be especially careful when upgrading to the popular 2.8 versions. A supported version will be released when issues become known.
- Installation: 
  - Download UnityMeshSync_Blender_*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases) 
    - When you decompress the file, there will be another zip file in the UnityMeshSync_Blender_* directory, but this can be left alone 
  - In Blender, go to File -> User Preferences -> Add-ons (2.80 and after: Edit -> User Preferences), click "Install Add-on from file" at the bottom of the screen, and select the plugin zip file. 
  - **If an older version is already installed, it must be deleted beforehand**. Select "Import-Export: Unity Mesh Sync" from the Add-ons menu, **restart Blender after removing the older version** then follow the above steps.
- "Import-Export: Unity Mesh Sync" will be added to the menu, so select it to enable it.
- The MeshSync will also be added, where settings and manual sync can be accessed. 
  - The panel's location can be difficult to find in 2.8 versions. Use the screenshot at the right for reference.
- When "Auto Sync" is selected, changes to the Mesh will automatically be reflected in Unity. If Auyo Sync is disabled, use the "Manual Sync" button to sync changes. 
- Pressing the Animations Sync button will cause the timer to advance from the first frame to the final frame while baking the animation, then send it to Unity. 

&nbsp;  

- Polygon mesh, camera, and lighting sync are supported.
- The polygon mesh's skinning/bone (Armature) and Blendshape will be sent to Unity unchanged. Mirror deformers are also supported. Other deformers will be ignored. 
  - Check "Bake Modifiers" to sync the results of applying all modifiers. This will make the Mesh content mostly consistent between  Blender and Unity, but will also result in the loss of Skinning and Blendshape information.  
- Use "Convert To Mesh" to convert objects such as Nurbs into polygons, if they are able to, then sync. 
- Check the "Double Sided" option to make the Mesh double-sided in Unity.
- Be advised that the negative scale is only supported for certain elements.
  - If XYZ all have negative values, the Mesh will sync properly, however if only one axis has a negative value Unity will treat the Mesh as though every axis has a negative value.


### Modo

  <img src="https://user-images.githubusercontent.com/1488611/55697991-d9135980-59fe-11e9-8e9f-8fcfba1b234f.png" height=300><img src="https://user-images.githubusercontent.com/1488611/55697990-d9135980-59fe-11e9-9312-29c95e20e5b0.png" height=300>

  Functionality confirmed with Modo 10, 12, 13 + Windows, Mac, Linux (CentOS 7).
  - Installation:
    - Download UnityMeshSync_Modo_*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases)
    - Designate MeshSyncClientModo.fx in Modo under System -> Add Plug-in
  - After installing, View will be added to the menu (Application -> Custom View -> UnityMeshSync), where varous options and settings can be accessed 
  - While "Auto Sync" is checked, changes made to the mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to sync changes
  - Clicking Sync under Animations will cause the timer to advance from the first frame to the final frame while baking the animation and sending it to Unity. 

  &nbsp;

  - Polygon mesh, camera, and light sync are supported. Portions of Mesh Instance and Replicator are also supported.
  - Polygon mesh Skinning/Joints and Morph will carry over to Unity, but be aware of how deformers are handled.
    - MeshSync can only handle Joint + Weight Map skinning, or Morph deformers. Any other deformers will be ignored.
    - Checking "Bake Deformers" will send the results of applying all deformers to Unity. This will mostly synchronize the Mesh on the Unity side even with complex deformer compositions, but comes at the cost of losing skinning and Morph/Blendshape information. 
    - Mesh Instance and Replicator skinning won't display properly in Unity. "Bake Deformers" must be used.
  - Clicking "Double Sided" will cause the Mesh to be double-sided in Unity. 
  - Be advised that the negative scale is only supported for certain elements.
    - If XYZ all have negative values, the Mesh will sync properly, however if only one axis has a negative value Unity will treat the Mesh as though every axis has a negative value.
  - MeshSync features can also be accessed via commands. Use unity.meshsync.settings to change settings, and unity.meshsync.export to export

  &nbsp;

As of Modo 13, the  [Mood Bridge for Unity](https://learn.foundry.com/modo/content/help/pages/appendices/modo_bridge.html) feature is available. This feature allows you to send Meshes and Materials directly to Unity.It has elements that are similar to MeshSync's features, with the following differences (as of 04/2019). 
  - Mood Bridge supports Modo <-> Unity sync in both directions. MeshSync only supports Modo -> Unity sync.
  - MeshSync can sync Replicator and Mesh Skinning/Morphs, and animations. Currently, Mood Bridge cannot.
  - MeshSync attempts to replicate the results of bringing data into Unity via FBX as much as possible. On the other hand Modo Bridge has big differences such as using a different coordinate system (Z direction is reversed), decompressing the Mesh index (a model with 1,000 triangles will have 3,000 vertices), etc.   


### Metasequoia
Supported in Windows for version 3 and 4 (32bit & 64bit) and Mac (version 4 only). All 3 versions are probably supported, but 4 versions must be 4.6.4 or later (bone output is not supported for earlier versions). 
Also, dll is different in version 4.7 and later. This is due to changes to the bone system after 4.7 which lead to a loss of plugin compatibility. Morph output is also supported in 4.7 and later. 
- Installation:
  - Download  UnityMeshSync_Metasequoia*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases) and decompress
  - Go to Help -> About Plug-ins in Metasequoia, and select the plugin file under "Install" in the lower left of the dialogue. It's a Station plugin type. 
  - **If older versions are already installed, remove them manually before hand**. Delete the appropriate files before starting Metasequoia. 
- After installation Panel -> Unity Mesh Sync will be added, open this and check "Auto Sync".
- While "Auto Sync" is checked, changes to the mesh will automatically be reflected in Unity. If Auto Sync is disabled, use the "Manual Sync" button to sync changes. 
- Checking "Double Sided" will cause the Mesh to be double-sided in Unity.
- Checking "Sync Camera" will sync the camera in Metasequoia. "Camera Path" is the camera path in Unity.
- Clicking "Import Unity Scene" will import the currently open Unity scene. Changes made to the scene can be reflected in real time. 

&nbsp;

- Mirroring and smooting will be reflected in Unity.
  - However, "reflective surfaces where the left and right are connected" type mirroring is not supported.
- Hidden objects in Metasequoia will also be hidden in Unity. Mesh details for hidden objects will not be sent to Unity, so when the number of objects in a scene makes sync heavy, making them hidden as appropriate should also speed up the sync process.  
- Materials will not be reflected in Unity, but they will be split into appropriate sub-meshes depending on the Material ID. 
- Subdivisions and metaballs will not be reflected in Unity until you freeze them. 
- Check "Sync Normals" to reflect vector changes supported by Metasequoia 4 versions. 
- Check "Sync Bones" to reflect bones supported by Metasequoia 4 versions. Checking "Sync Poses" will reflect the pose designated under "Skinning". 


### VRED
VRED has a special implementation using an API hook. This means that it also has special use methods and that minor version updates to VRED can result in loss of functionality. 
- Installation:
  - Download UnityMeshSync_VRED_Windows.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases) and extract files
  - If the VRED install destination is set to default, double clicking bat files like VREDPro_2019.bat will apply MeshSync and start VRED. 
  - If the install destination is NOT the default, open the bat file in a text editor and overwrite the "%PROGRAMFILES%\Autodesk\VREDPro-11.0\Bin\WIN64\VREDPro.exe" portion to point to the install destination's VRED exe file (Ex:"D:\My Install Directory\VREDPro-11.0\Bin\WIN64\VREDPro.exe").  
- While "Auto Sync" is checked, changes to the mesh will automatically be reflected in Unity. If Auto Sync is disabled, use the "Manual Sync" button to sync changes.
- Some portions of texture sync are supported. The textures designated in the "colorTexture" and "bumpTexture" parameters in the shader will be synced as textures for the color texture, normal map, etc.  
- Checking "Flip U/V" will invert the U/Vs of the texture coordinates. Even if the mapping method doesn't use U/Vs, this option can still help maintain consistency. 
- Clicking "Double Sided" will cause the Mesh to be double-sided in Unity.
- Clicking "Sync Deleted" will delete models in Unity that have been deleted in VRED.
- Clicking "Sync Camera" will sync the VRED camera. "Camera Path" is the Unity camera path.

&nbsp;  
  
- The model in the VRED viewport will be sent to Unity as is, so models can generally be synched.
  - However objects outside the camera's view will not be synched, due to VRED's culling. 
- Due to implementation restrictions, object names and tree structures cannot be synched. They will all become root objects in Unity, and the names will change automatically. 

### xismo
xismo doesn't provide a plugin, so the current implementation (05/2018) uses an API hook.This means that it also has special use methods and that minor version updates to xismo can result in loss of functionality. The current version of MeshSync is confirmed to work with xismo 191～199b.
- Installation:
  - Download UnityMeshSync_xismo_Windows.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases) 
  - Decompress it and place the 2 files (MeshSyncClientXismo.exe, MeshSyncClientXismoHook.dll) in the directory where xismo is installed (same place as xismo.exe)
- Run MeshSyncClientXismo.exe. xismo will start with MeshSync included.
- A window->Unity Mesh Sync menu will be added, where various settings can be accessed.
- While "Auto Sync" is checked, changes to the mesh will automatically be reflected in Unity.
- Clicking "Double Sided" will cause the Mesh to be double-sided in Unity.
- Clicking "Sync Camera" will sync the xismo camera. "Camera Path" is the Unity camera path.

&nbsp;  

- The model in the xismo viewport will be sent to Unity as is, so models can generally be synched.
- Non-model elements (object/material names, bones, etc.) are not supported. Support won't be possible until xismo releases a plugin API, so there are currently no plans to support these elements in MeshSync.  


### Unity
- Functionality confirmed in Unity 5.6 and later + Windows (64 bit), Mac, Linux (CentOS 7) 
- Installation:
  - Download MeshSync.unitypackage from [releases](https://github.com/unity3d-jp/MeshSync/releases) and import it into the project.
    - This repository can be directly imported into Unity 2018.3 and later versions. Open the project's Packages/manifest.json in a text editor and add the following line under "dependencies".
    > "com.utj.meshsync": "https://github.com/unity3d-jp/MeshSync.git",
  
  - When installing on older versions, **close Unity and delete Assets/UTJ/MeshSync before importing the package**. If the plugin dll loads, the update will fail. 
  - After importing, the GameObject -> MeshSync menu will be added
- Create a server object under GameObject -> MeshSync -> Create Server. This object will handle the sync process.

<img align="right" src="https://user-images.githubusercontent.com/1488611/49274442-c40c4400-f4bb-11e8-99d6-3257cdbe7320.png" height=400>

- Root Object
  - Designate an object to act as the root for object groups generated by synching. If not designated, an object will be created in the root. 
- Sync Transform, etc.
  - Setting for enabling/disabling sync for individual components. Created because of situations such as Transform sync getting in the way of checking physics simulation behavior in Play mode.
- Update Mesh Collider
  - When enabled, MeshCollidor details will also be updated when the Mesh is updated, if an object has a MeshCollider. 
- Track Material Assignment
  - When enabled, if a material is assigned by dragging and dropping in the SceneView, that update will be detected and any other Mesh using the same material will also have its material updated. 
- Animation Interpolation
  - Sets the animation interpolation method. The default smooth interpolation will be fine in most cases, but for films it may be preferable to match the number of animation samples to the target framerate on the DCC side and disable interpolation by selecting (Constant).   
- Progressive Display
  - When enabled, mid-reception scene updates will be reflected in real-time. When disabled, updates will be reflected after all of the scene data is received.  
  
  &nbsp;

- Material List
  - The MeshSyncServer maintains a list of materials. Assigning a material to this list will also assign it to corresponding objects as appropriate. 
- Make Asset
  - Meshes created by editing in DCC tools become objects that can only exist as they are in the scene where they are created. They have to be saved as asset files in order to be used in other scenes or projects. 
  Click the "Export Mesh "button in the MeshSyncServer to make those meshes into assets (files will be created in the directory designated by the "Asset Export Path").  


## Tips and Important Points
- Sync happens via TCP/IP, so it can still be done even if Unity and the DCC tool are on separate machines. To set up this kind of connection go to the client DCC tool's Server / Port setting and designate the machine running Unity.


- When there is a MeshSyncServer object in the scene in Unity, open the server's address:port in your browser to view the server's Unity GameView via the browser(the default is [127.0.0.1:8080](http://127.0.0.1:8080)). 
  If a message is sent from the browser's message form, that message will appear in Unity's Console. This communication method can be useful when Unity and the DCC tool are being used by different people. 


- If only the pose/animation are being edited, we recommend disabling "Sync Meshes". Not sending mesh data can make performance lighter.
  - If only the bones are being edited on the plugin side the mesh shouldn't be sent, but this can be difficult to judge so currently that doesn't happen by default.


- Note that in versions prior to Unity 2019.1 the most effective number of bones per vertex is 4. 
  If there are too many bones, the results won't sync between the DCC tool and Unity. 
    - In Unity 2019.1 a maximum of 255 bones can be used (See "Unlimited" in Skin Weights under Project Settings -> Quality -> Other).
    This means that cases which require lots of bones, such as facial animations, should be able to sync with no problem.


- 本プラグインはその性質上エディタでのみの使用を想定していますが、一応ランタイムでも動作するようにしてあります。**意図せず最終ビルドに残さないようご注意ください**。  
  ランタイムではアニメーションの同期は機能しませんが、モデルの同期は一通り動作します。


##  関連
- [NormalPainter](https://github.com/unity3d-jp/NormalPainter): Unity 上で法線を編集できるようにするツール
- [BlendShapeBuilder](https://github.com/unity3d-jp/BlendShapeBuilder): Unity 上で BlendShape を構築できるようにするツール

## ライセンス
[MIT](LICENSE.txt), ただし Blender プラグインは [GPL3](Plugin/MeshSyncClientBlender/LICENSE.txt) (Blender のソースの一部を使っているため)
