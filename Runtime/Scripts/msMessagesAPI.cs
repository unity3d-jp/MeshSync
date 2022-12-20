using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace Unity.MeshSync {
/// <summary>
/// The type of messages that can be sent to MeshSyncServer 
/// </summary>
public enum NetworkMessageType {
    /// <summary>
    /// An unknown message
    /// </summary>
    Unknown,

    /// <summary>
    /// A get message
    /// </summary>
    Get,

    /// <summary>
    /// A Set message
    /// </summary>
    Set,

    /// <summary>
    /// A delete message
    /// </summary>
    Delete,

    /// <summary>
    /// A fence message
    /// </summary>
    Fence,

    /// <summary>
    /// A text message
    /// </summary>
    Text,

    /// <summary>
    /// A screenshot message
    /// </summary>
    Screenshot,

    /// <summary>
    /// A query message
    /// </summary>
    Query,

    /// <summary>
    /// A response message
    /// </summary>
    Response,

    /// <summary>
    /// A message to send data from Unity back to a DCC tool
    /// </summary>
    RequestServerLiveEdit,

    /// <summary>
    /// A message to execute a Unity Editor command
    /// </summary>
    EditorCommand
}

//----------------------------------------------------------------------------------------------------------------------

internal struct GetMessage {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern GetFlags msGetGetFlags(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msGetGetBakeSkin(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msGetGetBakeCloth(IntPtr self);

    #endregion

    public static explicit operator GetMessage(IntPtr v) {
        GetMessage ret;
        ret.self = v;
        return ret;
    }

    internal GetFlags flags {
        get { return msGetGetFlags(self); }
    }

    public bool bakeSkin {
        get { return msGetGetBakeSkin(self) != 0; }
    }

    public bool bakeCloth {
        get { return msGetGetBakeCloth(self) != 0; }
    }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct SetMessage {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern SceneData msSetGetSceneData(IntPtr self);

    #endregion

    public static explicit operator SetMessage(IntPtr v) {
        SetMessage ret;
        ret.self = v;
        return ret;
    }

    public SceneData scene {
        get { return msSetGetSceneData(self); }
    }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct DeleteMessage {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern int msDeleteGetNumEntities(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Identifier msDeleteGetEntity(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern int msDeleteGetNumMaterials(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Identifier msDeleteGetMaterial(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern int msDeleteGetNumInstances(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Identifier msDeleteGetInstance(IntPtr self, int i);

    #endregion

    public static explicit operator DeleteMessage(IntPtr v) {
        DeleteMessage ret;
        ret.self = v;
        return ret;
    }

    public int numEntities {
        get { return msDeleteGetNumEntities(self); }
    }

    internal Identifier GetEntity(int i) {
        return msDeleteGetEntity(self, i);
    }

    public int numMaterials {
        get { return msDeleteGetNumMaterials(self); }
    }

    internal Identifier GetMaterial(int i) {
        return msDeleteGetMaterial(self, i);
    }

    public int numInstances {
        get { return msDeleteGetNumInstances(self); }
    }

    internal Identifier GetInstance(int i) {
        return msDeleteGetInstance(self, i);
    }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct FenceMessage {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern FenceType msFenceGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msFenceGetDCCToolName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msMessageGetSessionID(IntPtr self);

    #endregion

    public enum FenceType {
        Unknown,
        SceneBegin,
        SceneEnd
    }

    public static explicit operator FenceMessage(IntPtr v) {
        FenceMessage ret;
        ret.self = v;
        return ret;
    }

    public FenceType type {
        get { return msFenceGetType(self); }
    }

    public int SessionId {
        get { return msMessageGetSessionID(self); }
    }

    internal string DCCToolName {
        get {
            // The method may not exist, we don't want to bump the protocol version just for this change:
            try {
                IntPtr
                    nativeStr = msFenceGetDCCToolName(
                        self); //Not direct marshalling because there is no free on C# side.
                return  Marshal.PtrToStringAnsi(nativeStr);
            }
            catch {
                return "";
            }
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

internal struct TextMessage {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern IntPtr msTextGetText(IntPtr self);

    [DllImport(Lib.name)]
    private static extern TextType msTextGetType(IntPtr self);

    #endregion

    public enum TextType {
        Normal,
        Warning,
        Error
    }

    public static explicit operator TextMessage(IntPtr v) {
        TextMessage ret;
        ret.self = v;
        return ret;
    }

    public string text {
        get { return Misc.S(msTextGetText(self)); }
    }

    public TextType textType {
        get { return msTextGetType(self); }
    }

    public void Print() {
        switch (textType) {
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

internal struct QueryMessage {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern QueryType msQueryGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msQueryFinishRespond(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msQueryAddResponseText(IntPtr self, string text);

    #endregion

    public enum QueryType {
        Unknown,
        PluginVersion,
        ProtocolVersion,
        HostName,
        RootNodes,
        AllNodes
    }

    public static explicit operator QueryMessage(IntPtr v) {
        QueryMessage ret;
        ret.self = v;
        return ret;
    }

    public QueryType queryType {
        get { return msQueryGetType(self); }
    }

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
        SceneUpdate
    }
}

internal struct EditorCommandMessage {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern CommandType msEditorCommandGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msEditorCommandGetId(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msEditorCommandGetSession(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msEditorCommandGetBuffer(IntPtr self);

    #endregion

    public static explicit operator EditorCommandMessage(IntPtr v) {
        EditorCommandMessage ret;
        ret.self = v;
        return ret;
    }

    public enum CommandType {
        Unknown,
        AddServerToScene,
        GetProjectPath
    }

    public CommandType commandType {
        get { return msEditorCommandGetType(self); }
    }

    public int id {
        get { return msEditorCommandGetId(self); }
    }

    public int session {
        get { return msEditorCommandGetSession(self); }
    }

    public string buffer {
        get { return Misc.S(msEditorCommandGetBuffer(self)); }
    }
}
} //end namespace