using System;

namespace Unity.MeshSync {

/// <summary>
/// A callback that is called when receiving message data from server 
/// </summary>
public delegate void ServerMessageCallback(MessageType type, IntPtr data);

} //end namespace
