# Bidirectional Sync

For [MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest) of DCC tools that have Bidirectional Sync support, 
we can change editable properties in Unity and MeshSync will apply them in the DCC tool.
  

![](images/MeshSyncServerLiveEditProperties.png)

When applicable, MeshSync will add a MeshSyncServerLiveEditProperties component to the [MeshSyncServer](MeshSyncServer.md) **GameObject**
and put the editable properties in it.

Please refer to the documentation of [MeshSyncDCCPlugins](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
for more details on these properties.
