# Bidirectional Sync

[MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
for certain DCC tools have Bidirectional Sync feature that allows us to apply property changes in Unity back to the DCC tool.  

![](images/MeshSyncServerLiveEditProperties.png)

When applicable, MeshSync will add a MeshSyncServerLiveEditProperties component to the [MeshSyncServer](MeshSyncServer.md) **GameObject**
and put the editable properties in it.

Please refer to the documentation of [MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
for more details on these properties.
