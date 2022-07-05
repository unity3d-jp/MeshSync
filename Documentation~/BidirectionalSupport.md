# Unity <-> Blender Bidirectional support

This feature allows you to make changes in blender from inside Unity.

## Enabling bidirectional support
Turn on auto-sync in blender to enable bidirectional support.

## Custom properties and geometry node parameters
Unity displays blender object's custom properties and geometry node modifier values and allows changing them from inside Unity.
These properties are displayed in the inspector of a MeshSyncServerProperties MonoBehaviour below the MeshSyncServer.
The supported property types are:
* Integer
* Float
* Integer Array
* Float Array
* String (read-only)
This feature is intended to be used to easily change blender's procedural generation parameters from inside Unity.
Note: This feature requires blender version 3 or higher.

## Meshes
If ProBuilder is installed in the Unity project, the MeshSyncServer has an option to use ProBuilder Meshes.
These can be edited in Unity and are sent back to blender. 
This feature is intended to be used to easily change geometry node mesh inputs from inside Unity.
Due to the way Unity represents the mesh, it is triangulated when it's sent back to blender and some mesh data like vertex groups and shape keys, etc. may get lost. If the option to bake modifiers was enabled in blender, a mesh that is sent from Unity back into blender will have those modifiers applied and removed.

## Splines
If the Unity Splines package version 2 is installed in the Unity project and "Curves as Mesh" is disbled in blender, the MeshSyncServer in Unity creates splines from blender curves. These can be edited in Unity and are sent back to blender.
This feature is intended to be used to easily change geometry node curve inputs from inside Unity.

## Instance handling
Instances of blender objects can be created in Unity in different ways, this is an option on the MeshSyncServer in Unity:
* Instance renderer
	* Creates a MeshSync specific render component that batches the draw calls of each instance type. This is good for instances that do not need individual GameObjects.
* Copies
	* Creates a GameObject for each instance. This is slower than the instance renderer but gives more control over each object.
* Prefabs
	* Creates a prefab of the instanced object and places prefab instances in the scene. The prefab is saved in the 'Asset Dir' set on the MeshSyncServer. Using prefabs for instances allows you to make changes to the instanced object and add your own MonoBehaviours to it and apply that to all instances. 
	Note that prefabs are referenced by their blender name, if the name in blender changes, they are considered to be a new object and a new prefab is created.	
	Prefabs are not overwritten when their blender source object changes. 
	There is a "Clear prefabs" button to delete all prefabs for this server and regenerate them if blender is open and auto-sync is enabled.

## DCC Asset setting
The MeshSyncServer has a setting for an DCC asset file. This allows you to set a .blend file from the asset library, pressing 'Live Edit' beside it, opens the selected blender version and turns on MeshSync's Auto-Sync feature.
There are multiplee options for the Run mode:
* GUI
	* Runs blender in normal window mode.
* Background
	* Runs blender in headless mode without any window.
* Console
	* Runs blender with the console window only.

## Tips
For some files to sync correctly, the Bake Modifiers option must be selected.
![Menu](images/BakeModsTip.png)
