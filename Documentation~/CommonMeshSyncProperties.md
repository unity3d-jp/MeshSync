# Common MeshSync Properties

These properties are shared by multiple components in this package.

1. [Asset Sync Settings](#asset-sync-settings)
1. [Import](#import)
1. [Misc](#misc)
1. [Material List](#material-list)
1. [Animation Tweaks](#animation-tweaks)
1. [Export Assets](#export-assets)

## Asset Sync Settings

![](images/MeshSyncAssetSyncProperties.png)

|**Properties** |**Description** |
|:---       |:---|
| Update Transform      | updates the position, rotation, and scale of each **GameObject**.|
| Cameras               | **Create**: create camera **GameObjects**. <br/> **Update**: update camera properties.|
| Use Physical Params   | update physical camera properties.|
| Cameras               | **Create**: create light **GameObjects**. <br/> **Update**: update light properties.|
| Meshes                | create and update the MeshRenderer or SkinnedMeshRenderer component of applicable **GameObjects**.|
| Update Mesh Colliders | create and update MeshCollider properties of **GameObjects** which have meshes if the meshes are updated.|
| Visibility            | enable/disable relevant components based on the visibility flag in the source data.|

> Tips: disabling **Sync Meshes** brings better performance, and is recommended if syncing meshes is not necessary, 
> for example, when editing only the pose/animation.

## Import 
   
<img src='images/MeshSyncImportProperties.png' width='450'>

- **Create Materials**: finds existing materials based on their names using the **Search Mode**
  and applies them to relevant **GameObjects**.  
  If the material is not found, then a new material is created.  
  All materials are always added to the [material list](#material-list) of the component.

  |**Search Mode**  |**Description** |
  |:---             |:---|
  | Local           | search in the local folder of the MeshSync component.|
  | Recursive-Up    | search starting from the local folder of the MeshSync component up to the Assets folder.|
  | Everywhere      | search in all Unity Project folders.|
  
- **Overwrite Exported Materials**

   Apply changes from dcc tools to exported materials that are referenced in the material list.
   |**Values** |**Description** |
   |:---       |:---|
   | On| Apply changes to exported materials.|
   | Off| Do not apply changes to exported materials.|
   
- **Default Shader**

   Use this shader instead of the default for the current render pipeline when creating materials.

- **Animation Interpolation**: sets the animation interpolation method.   

  |**Values** |**Description** |
  |:---       |:---|
  | Smooth    | smoothly interpolate animation curves.|
  | Linear    | do linear interpolation between neighboring animation keys.|
  | Constant  | disable interpolation.|

  > For use cases in film productions, "Constant" may be preferable to match 
  > the number of animation samples to the target framerate on the DCC side.


- **Keyframe Reduction**: performs keyframe reduction when importing animations.  
  - **Threshold**: the error tolerance.   
    Higher threshold means less number of keys and higher errors, and vice versa.   
  - **Erase Flat Curves**: delete curves that have no change (flat).
  
- **Z-Up Correction**  
  Specifies how to convert Z-Up to Y-Up for data from DCC tools 
  which have Z-Up coordinate system (3ds Max, Blender, etc).

  |**Values**     |**Description** |
  |:---           |:---|
  | **Flip YZ**   | converts all vertices of Transform and Mesh to Y-Up.|
  | **Rotate X**  | converts the root object's Transform to a Y-Up by applying a -90 X axis rotation to the root object, leaving the mesh in Z-Up.|

  > **Flip YZ** works better in most cases.   
  > For reference, Unity's standard FBX Importer does the equivalent of **Rotate X**.

## Misc 
   
![](images/MeshSyncMiscProperties.png)

- **Sync Material List**  
  When enabled, changing an object's material in the Scene view will update the [material list](#material-list),
  and other objects which use the previous material will be updated to use the new material.

- **Progressive Display**  

  |**Values** |**Description** |
  |:---       |:---|
  | On        | scene updates will be reflected while receiving data in real-time.|
  | Off       | updates will be reflected after all of the scene data is received.|

## Material List
   
![](images/MeshSyncMaterialsProperties.png)

This material list holds all the materials that are used by this component.   
Changing a material in this list will update objects that use the previous material 
to use the new material.

**Import List** and **Export List** buttons are used to load and save material lists.

## Export Assets 

![](images/MeshSyncExportAssetsProperties.png)
  
Export meshes and materials into asset files, in order to reuse them in other scenes or projects.  
Normally, meshes and materials hold by this component only exist in the active scene.

