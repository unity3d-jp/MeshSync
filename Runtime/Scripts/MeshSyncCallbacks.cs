using System;

namespace Unity.MeshSync {

/// <summary>
/// This callback will be called when receiving message data from server, if it is set to MeshSyncServer.  
/// </summary>
public delegate void ServerMessageCallback(MessageType type, IntPtr data);

} //end namespace
