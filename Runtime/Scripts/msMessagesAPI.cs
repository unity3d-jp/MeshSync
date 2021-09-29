using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace Unity.MeshSync
{

/// <summary>
/// The type of messages that can be sent to MeshSyncServer 
/// </summary>
public enum MessageType {
    Unknown,
    Get,
    Set,
    Delete,
    Fence,
    Text,
    Screenshot,
    Query,
    Response,
}

//----------------------------------------------------------------------------------------------------------------------

internal struct GetMessage {
    #region internal
    public IntPtr self;
    [DllImport(Lib.name)] static extern GetFlags msGetGetFlags(IntPtr self);
    [DllImport(Lib.name)] static extern int msGetGetBakeSkin(IntPtr self);
    [DllImport(Lib.name)] static extern int msGetGetBakeCloth(IntPtr self);
    #endregion

    public static explicit operator GetMessage(IntPtr v) {
        GetMessage ret;
        ret.self = v;
        return ret;
    }

    internal GetFlags flags { get { return msGetGetFlags(self); } }
    public bool bakeSkin { get { return msGetGetBakeSkin(self) != 0; } }
    public bool bakeCloth { get { return msGetGetBakeCloth(self) != 0; } }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct SetMessage {
    #region internal
    public IntPtr self;
    [DllImport(Lib.name)] static extern SceneData msSetGetSceneData(IntPtr self);
    #endregion

    public static explicit operator SetMessage(IntPtr v)
    {
        SetMessage ret;
        ret.self = v;
        return ret;
    }

    public SceneData scene
    {
        get { return msSetGetSceneData(self); }
    }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct DeleteMessage {
    #region internal
    public IntPtr self;
    [DllImport(Lib.name)] static extern int msDeleteGetNumEntities(IntPtr self);
    [DllImport(Lib.name)] static extern Identifier msDeleteGetEntity(IntPtr self, int i);
    [DllImport(Lib.name)] static extern int msDeleteGetNumMaterials(IntPtr self);
    [DllImport(Lib.name)] static extern Identifier msDeleteGetMaterial(IntPtr self, int i);
    #endregion

    public static explicit operator DeleteMessage(IntPtr v) {
        DeleteMessage ret;
        ret.self = v;
        return ret;
    }

    public int numEntities { get { return msDeleteGetNumEntities(self); } }
    internal Identifier GetEntity(int i) { return msDeleteGetEntity(self, i); }

    public int numMaterials { get { return msDeleteGetNumMaterials(self); } }
    internal Identifier GetMaterial(int i) { return msDeleteGetMaterial(self, i); }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct FenceMessage {
    #region internal
    public IntPtr self;
    [DllImport(Lib.name)] static extern FenceType msFenceGetType(IntPtr self);
    #endregion

    public enum FenceType {
        Unknown,
        SceneBegin,
        SceneEnd,
    }

    public static explicit operator FenceMessage(IntPtr v) {
        FenceMessage ret;
        ret.self = v;
        return ret;
    }

    public FenceType type { get { return msFenceGetType(self); } }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct TextMessage {
    #region internal
    public IntPtr self;
    [DllImport(Lib.name)] static extern IntPtr msTextGetText(IntPtr self);
    [DllImport(Lib.name)] static extern TextType msTextGetType(IntPtr self);
    #endregion

    public enum TextType
    {
        Normal,
        Warning,
        Error,
    }

    public static explicit operator TextMessage(IntPtr v)
    {
        TextMessage ret;
        ret.self = v;
        return ret;
    }

    public string text { get { return Misc.S(msTextGetText(self)); } }
    public TextType textType { get { return msTextGetType(self); } }

    public void Print() {
        switch (textType)
        {
            case TextType.Error:
                Debug.LogError(text);
                break;
            case TextType.Warning:
                Debug.LogWarning(text);
                break;
            default:
                Debug.Log(text);
                break;
        }

    }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct QueryMessage
{
    #region internal
    public IntPtr self;
    [DllImport(Lib.name)] static extern QueryType msQueryGetType(IntPtr self);
    [DllImport(Lib.name)] static extern void msQueryFinishRespond(IntPtr self);
    [DllImport(Lib.name)] static extern void msQueryAddResponseText(IntPtr self, string text);
    #endregion

    public enum QueryType {
        Unknown,
        PluginVersion,
        ProtocolVersion,
        HostName,
        RootNodes,
        AllNodes,
    }

    public static explicit operator QueryMessage(IntPtr v) {
        QueryMessage ret;
        ret.self = v;
        return ret;
    }

    public QueryType queryType { get { return msQueryGetType(self); } }

    public void FinishRespond() {
        msQueryFinishRespond(self);
    }
    public void AddResponseText(string text) {
        msQueryAddResponseText(self, text);
    }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct PollMessage {
    public enum PollType {
        Unknown,
        SceneUpdate,
    }
}

} //end namespace
