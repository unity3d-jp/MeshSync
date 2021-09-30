using System;

namespace Unity.MeshSync {

/// <summary>
/// A callback that can be set to MeshSyncServer to be called when the server receives data   
/// </summary>
public delegate void ServerMessageCallback(NetworkMessageType type);

} //end namespace
