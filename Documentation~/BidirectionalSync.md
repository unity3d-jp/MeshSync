# Bidirectional Sync

[MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
for certain DCC tools have Bidirectional Sync feature that allows us to make changes in DCC Tools from inside Unity, 
and we can simply turn on "Auto Sync" in the plugin properties to enable this feature.

## Supported Parameters

![](images/MeshSyncServerLiveEditProperties.png)

Synchronizable properties are displayed in the inspector of a MeshSyncServerLiveEditProperties component, 
which is added automatically into the **GameObject** with [MeshSyncServer](MeshSyncServer.md) component.


## Meshes
If ProBuilder is installed in the Unity project, the MeshSyncServer has an option to use ProBuilder Meshes.
These can be edited in Unity and are sent back to blender. 
This feature is intended to be used to easily change geometry node mesh inputs from inside Unity.
Due to the way Unity represents the mesh, it is triangulated when it's sent back to blender 
and some mesh data like vertex groups and shape keys, etc. may get lost. 
If the option to bake modifiers was enabled in blender, 
a mesh that is sent from Unity back into blender will have those modifiers applied and removed.




---------------------


Unity displays blender object's custom properties and geometry node modifier values and allows changing them from inside Unity.

 below the MeshSyncServer.
The supported property types are:
* Integer
* Float
* Integer Array
* Float Array
* String (read-only)  
This feature is intended to be used to easily change blender's procedural generation parameters from inside Unity.
Note: This feature requires blender version 3 or higher.



