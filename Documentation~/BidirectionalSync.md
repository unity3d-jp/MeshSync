# Bidirectional Sync

[MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
for certain DCC tools have Bidirectional Sync feature that allows us to make changes in DCC Tools from inside Unity, 
and we can simply turn on "Auto Sync" in the plugin properties to enable this feature.

## Supported Parameters

![](images/MeshSyncServerLiveEditProperties.png)

Synchronizable properties are displayed in the inspector of a MeshSyncServerLiveEditProperties component, 
which is added automatically into the **GameObject** with [MeshSyncServer](MeshSyncServer.md) component.

Please refer to the documentation of [MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
for more details on these properties.

## Editing and Syncing Meshes from Unity to DCC Tool

We can edit and send meshes back to a DCC Tool if 
[ProBuilder](https://docs.unity3d.com/Packages/com.unity.probuilder@5.0/manual/index.html) is installed in the Unity project.

This feature is intended for changing geometry node mesh inputs from inside Unity easily.
Due to the way Unity represents a mesh, a mesh will be triangulated when it's sent back to the DCC tool 
and some mesh data like vertex groups and shape keys, etc. may get lost. 
If the option to bake modifiers (or its equivalent) is enabled in the DCC tool, 
a mesh that is sent from Unity back into the DCC tool will have those modifiers applied and removed.
