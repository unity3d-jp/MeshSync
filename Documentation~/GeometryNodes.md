# Geometry Nodes Synchronization

Geometry Nodes is a feature that is currently only provided by Blender and 
used for modifying the geometry of an object with node-based operations.  
Meshsync will automatically synchronize geometry node instances during the sync process
from the DCC tool to Unity.

## Operations

### Disabling Instances

To stop rendering instances of an object:
1. Cick on the **GameObject** corresponding to the Blender object that has the geometry node modifier which generates the instances. 
2. Find the MeshSyncInstanceRenderer component that has a reference to the instanced object. 
3. Disable the component to disable the instances.

![](images/GeometryNodesDisable.gif)

### Controlling instances

We can control the rendering and world transform of instances by modifying the **GameObject** that 
is being instantiated.

![Menu](images/GeometryNodesMove.gif)

Property changes in the following components are automatically applied on instances:

|**Components** |**Properties** |
|:---                 |:---|
| Transform           | * Position <br/> * Rotation <br/> * Scale <br/> * Layer|
| MeshRenderer        | * Cast Shadows  <br/> * Receive Shadows <br/> * Light Probes <br/> * Proxy Volume Override|
| SkinnedMeshRenderer | * Cast Shadows <br/> * Receive Shadows <br/> * Light Probes <br/> * Proxy Volume Override|

## Runtime-Builds

You can create Runtime builds that render instances from Geometry Nodes.  
Before building, make sure that the required instancing variants are not stripped by 
selecting the _Keep All_ or _Strip Unused_ option in _Project Settings_ &rarr; _Graphics_ &rarr; _Shader Stripping_ &rarr; _Instancing Variants_.

## Limitations

### Scene Cache
Exportation of instances to a Scene Cache file is not supported.

### Preview Window
Rendering instances on the inspector preview window is not supported.

### Creating prefabs
Creating prefabs for instances is not supported.

## Tips
For some files to sync correctly, the Bake Modifiers option must be selected.
![Menu](images/BakeModsTip.png)
