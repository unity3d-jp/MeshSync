using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Assertions;

#if AT_USE_SPLINES
using Unity.Mathematics;
#endif

namespace Unity.MeshSync {
#if UNITY_STANDALONE || UNITY_EDITOR

#region Server

internal struct ServerSettings {
    public struct Flags {
        public BitFlags flags;
    }


    public int    maxQueue;
    public int    maxThreads;
    public ushort port;

    public Flags flags; // reserved
    public uint  meshSplitUnit;
    public uint  meshMaxBoneInfluence; // 4 or 255 (variable)

    public static ServerSettings defaultValue {
        get {
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateInstance();
            ServerSettings ret = new ServerSettings {
                maxQueue             = 512,
                maxThreads           = 8,
                port                 = settings.GetDefaultServerPort(),
                meshSplitUnit        = Lib.maxVerticesPerMesh,
                meshMaxBoneInfluence = Lib.maxBoneInfluence
            };
            return ret;
        }
    }
}

internal struct Server {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern Server msServerIsStarted(int port);

    [DllImport(Lib.name)]
    private static extern Server msServerStart(ref ServerSettings settings);

    [DllImport(Lib.name)]
    private static extern void msServerStop(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msServerAbort(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msServerSetZUpCorrectionMode(IntPtr self, ZUpCorrectionMode v);

    [DllImport(Lib.name)]
    private static extern int msServerGetNumMessages(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msServerProcessMessages(IntPtr self, MessageHandler handler);

    [DllImport(Lib.name)]
    private static extern void msServerBeginServe(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msServerEndServe(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msServerServeTransform(IntPtr self, TransformData data);

    [DllImport(Lib.name)]
    private static extern void msServerServeCamera(IntPtr self, CameraData data);

    [DllImport(Lib.name)]
    private static extern void msServerServeLight(IntPtr self, LightData data);

    [DllImport(Lib.name)]
    private static extern void msServerServeMesh(IntPtr self, MeshData data);

    [DllImport(Lib.name)]
    private static extern void msServerServeTexture(IntPtr self, TextureData data);

    [DllImport(Lib.name)]
    private static extern void msServerServeMaterial(IntPtr self, MaterialData data);

    [DllImport(Lib.name)]
    private static extern void msServerAllowPublicAccess(IntPtr self, bool access);

    [DllImport(Lib.name)]
    private static extern bool msServerIsPublicAccessAllowed(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msServerSetFileRootPath(IntPtr self, string path);

    [DllImport(Lib.name)]
    private static extern void msServerSetScreenshotFilePath(IntPtr self, string path);

    [DllImport(Lib.name)]
    private static extern void msServerNotifyPoll(IntPtr self, PollMessage.PollType t);

    [DllImport(Lib.name)]
    private static extern void msServerSendPropertyInt(IntPtr self, int sourceType, string name, string path, string modifierName, string propertyName,
        int newValue);

    [DllImport(Lib.name)]
    private static extern void msServerSendPropertyFloat(IntPtr self, int sourceType, string name, string path, string modifierName, string propertyName,
        float newValue);

    [DllImport(Lib.name)]
    private static extern void msServerSendPropertyIntArray(IntPtr self, int sourceType, string name, string path, string modifierName, string propertyName,
        int[] newValue, int arrayLength);

    [DllImport(Lib.name)]
    private static extern void msServerSendPropertyFloatArray(IntPtr self, int sourceType, string name, string path, string modifierName, string propertyName,
        float[] newValue, int arrayLength);

    [DllImport(Lib.name)]
    private static extern void msServerSendPropertyString(IntPtr self, int sourceType, string name, string path, string modifierName, string propertyName,
        string newValue, int length);

#if AT_USE_SPLINES
    [DllImport(Lib.name)]
    static extern void msServerSendTransform(IntPtr self, string path, float3 position, float3 scale, float3 rotation);  

    [DllImport(Lib.name)]
    static extern void msServerSendCurve(IntPtr self, string path, int splineIndex, int knotCount, bool closed, float3[] cos, float3[] handlesLeft, float3[] handlesRight);
#endif

#if AT_USE_PROBUILDER
    [DllImport(Lib.name)]
    private static extern void msServerSendMesh(IntPtr self, MeshData data);
#endif

    [DllImport(Lib.name)]
    private static extern void msServerRequestFullSync(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msServerRequestUserScriptCallback(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msServerInitiatedResponseReady(IntPtr self);

    [DllImport(Lib.name)]
    private static extern bool msServerIsDCCLiveEditReady(IntPtr self);


    [DllImport(Lib.name)]
    private static extern void msServerNotifyEditorCommand(IntPtr self, string reply, int messageId, int sessionId);

    #endregion

    public delegate void MessageHandler(NetworkMessageType type, IntPtr data);

    public static implicit operator bool(Server v) {
        return v.self != IntPtr.Zero;
    }

    internal static bool IsStarted(int port) {
        return msServerIsStarted(port);
    }

    public static bool Start(ref ServerSettings settings, out Server server) {
        server = msServerStart(ref settings);
        return server;
    }

    public void Stop() {
        msServerStop(self);
    }

    public void Abort() {
        msServerAbort(self);
    }

    internal void SetZUpCorrectionMode(ZUpCorrectionMode mode) {
        msServerSetZUpCorrectionMode(self, mode);
    }

    public int numMessages {
        get { return msServerGetNumMessages(self); }
    }

    public void ProcessMessages(MessageHandler handler) {
        msServerProcessMessages(self, handler);
    }

    public string fileRootPath {
        set { msServerSetFileRootPath(self, value); }
    }

    public void AllowPublicAccess(bool access) {
        Assert.IsFalse(self == IntPtr.Zero);
        msServerAllowPublicAccess(self, access);
    }

    public bool IsPublicAccessAllowed() {
        Assert.IsFalse(self == IntPtr.Zero);
        return msServerIsPublicAccessAllowed(self);
    }


    public string screenshotPath {
        set { msServerSetScreenshotFilePath(self, value); }
    }


#if AT_USE_SPLINES
        public void SendCurve(string path, int splineIndex, int knotCount, bool closed, float3[] cos, float3[] handlesLeft, float3[] handlesRight)
    {
        msServerSendCurve(self, path, splineIndex, knotCount, closed, cos, handlesLeft, handlesRight);
    }
#endif

#if AT_USE_PROBUILDER
    public void SendMesh(MeshData data) {
        msServerSendMesh(self, data);
    }
#endif

    public void SendProperty(PropertyInfoDataWrapper prop) {
        switch (prop.type) {
            case PropertyInfoDataType.Int:
                msServerSendPropertyInt(self, (int)prop.sourceType, prop.name, prop.path, prop.modifierName, prop.propertyName, prop.GetValue<int>());
                break;

            case PropertyInfoDataType.Float:
                msServerSendPropertyFloat(self, (int)prop.sourceType, prop.name, prop.path, prop.modifierName, prop.propertyName, prop.GetValue<float>());
                break;

            case PropertyInfoDataType.IntArray:
                msServerSendPropertyIntArray(self, (int)prop.sourceType, prop.name, prop.path, prop.modifierName, prop.propertyName, prop.GetValue<int[]>(),
                    prop.arrayLength);
                break;

            case PropertyInfoDataType.FloatArray:
                msServerSendPropertyFloatArray(self, (int)prop.sourceType, prop.name, prop.path, prop.modifierName, prop.propertyName, prop.GetValue<float[]>(),
                    prop.arrayLength);
                break;

            case PropertyInfoDataType.String:
                string s = prop.GetValue<string>();
                msServerSendPropertyString(self, (int)prop.sourceType, prop.name, prop.path, prop.modifierName, prop.propertyName, s, s.Length);
                break;

            default:
                throw new NotImplementedException($"Type {prop.type} not implemented");
        }
    }

    public void RequestClientSync() {
        msServerRequestFullSync(self);
    }

    public void RequestUserScriptCallback() {
        msServerRequestUserScriptCallback(self);
    }

    public void MarkServerInitiatedResponseReady() {
        msServerInitiatedResponseReady(self);
    }

    public bool IsDCCLiveEditReady() {
        return msServerIsDCCLiveEditReady(self);
    }

    public void BeginServe() {
        msServerBeginServe(self);
    }

    public void EndServe() {
        msServerEndServe(self);
    }

    public void ServeTransform(TransformData data) {
        msServerServeTransform(self, data);
    }

    public void ServeCamera(CameraData data) {
        msServerServeCamera(self, data);
    }

    public void ServeLight(LightData data) {
        msServerServeLight(self, data);
    }

    public void ServeMesh(MeshData data) {
        msServerServeMesh(self, data);
    }

    public void ServeTexture(TextureData data) {
        msServerServeTexture(self, data);
    }

    public void ServeMaterial(MaterialData data) {
        msServerServeMaterial(self, data);
    }

    public void NotifyPoll(PollMessage.PollType t) {
        msServerNotifyPoll(self, t);
    }

    public void NotifyEditorCommand(string reply, EditorCommandMessage message) {
        msServerNotifyEditorCommand(self, reply, message.id, message.session);
    }
}

#endregion


#endif // UNITY_STANDALONE || UNITY_EDITOR
}      //end namespace