using System;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Assertions;

namespace Unity.MeshSync
{
#if UNITY_STANDALONE
    #region Server
    internal struct ServerSettings
    {
        public struct Flags
        {
            public BitFlags flags;
        }


        public int maxQueue;
        public int maxThreads;
        public ushort port;

        public Flags flags; // reserved
        public uint meshSplitUnit;
        public uint meshMaxBoneInfluence; // 4 or 255 (variable)
        public ZUpCorrectionMode zUpCorrectionMode;

        public static ServerSettings defaultValue
        {
            get {
                MeshSyncRuntimeSettings settings = MeshSyncRuntimeSettings.GetOrCreateSettings();
                ServerSettings ret = new ServerSettings {
                    maxQueue = 512,
                    maxThreads = 8,
                    port = settings.GetDefaultServerPort(),
                    meshSplitUnit = Lib.maxVerticesPerMesh,
                    meshMaxBoneInfluence = Lib.maxBoneInfluence,
                    zUpCorrectionMode = ZUpCorrectionMode.FlipYZ,
                };
                return ret;
            }
        }
    }

    internal struct Server
    {
        #region internal
        public IntPtr self;
        [DllImport(Lib.name)] static extern Server msServerStart(ref ServerSettings settings);
        [DllImport(Lib.name)] static extern void msServerStop(IntPtr self);

        [DllImport(Lib.name)] static extern void msServerSetZUpCorrectionMode(IntPtr self, ZUpCorrectionMode v);

        [DllImport(Lib.name)] static extern int msServerGetNumMessages(IntPtr self);
        [DllImport(Lib.name)] static extern int msServerProcessMessages(IntPtr self, MessageHandler handler);

        [DllImport(Lib.name)] static extern void msServerBeginServe(IntPtr self);
        [DllImport(Lib.name)] static extern void msServerEndServe(IntPtr self);
        [DllImport(Lib.name)] static extern void msServerServeTransform(IntPtr self, TransformData data);
        [DllImport(Lib.name)] static extern void msServerServeCamera(IntPtr self, CameraData data);
        [DllImport(Lib.name)] static extern void msServerServeLight(IntPtr self, LightData data);
        [DllImport(Lib.name)] static extern void msServerServeMesh(IntPtr self, MeshData data);
        [DllImport(Lib.name)] static extern void msServerServeTexture(IntPtr self, TextureData data);
        [DllImport(Lib.name)] static extern void msServerServeMaterial(IntPtr self, MaterialData data);
        [DllImport(Lib.name)] static extern void msServerAllowPublicAccess(IntPtr self, bool access);
        [DllImport(Lib.name)] static extern bool msServerIsPublicAccessAllowed(IntPtr self);
        [DllImport(Lib.name)] static extern void msServerSetFileRootPath(IntPtr self, string path);
        
        [DllImport(Lib.name)] static extern void msServerSetScreenshotFilePath(IntPtr self, string path);
        [DllImport(Lib.name)] static extern void msServerNotifyPoll(IntPtr self, PollMessage.PollType t);
        #endregion

        public delegate void MessageHandler(MessageType type, IntPtr data);

        public static implicit operator bool(Server v) { return v.self != IntPtr.Zero; }

        public static Server Start(ref ServerSettings settings) { return msServerStart(ref settings); }
        public void Stop() { msServerStop(self); }

        internal ZUpCorrectionMode zUpCorrectionMode
        {
            set { msServerSetZUpCorrectionMode(self, value); }
        }

        public int numMessages { get { return msServerGetNumMessages(self); } }
        public void ProcessMessages(MessageHandler handler) { msServerProcessMessages(self, handler); }

        public string fileRootPath { set { msServerSetFileRootPath(self, value); } }

        public void AllowPublicAccess(bool access) {
            Assert.IsFalse(self == IntPtr.Zero);
            msServerAllowPublicAccess(self, access);
        }

        public bool IsPublicAccessAllowed() {
            Assert.IsFalse(self == IntPtr.Zero);
            return msServerIsPublicAccessAllowed(self);
        }

        
        public string screenshotPath { set { msServerSetScreenshotFilePath(self, value); } }

        public void BeginServe() { msServerBeginServe(self); }
        public void EndServe() { msServerEndServe(self); }
        public void ServeTransform(TransformData data) { msServerServeTransform(self, data); }
        public void ServeCamera(CameraData data) { msServerServeCamera(self, data); }
        public void ServeLight(LightData data) { msServerServeLight(self, data); }
        public void ServeMesh(MeshData data) { msServerServeMesh(self, data); }
        public void ServeTexture(TextureData data) { msServerServeTexture(self, data); }
        public void ServeMaterial(MaterialData data) { msServerServeMaterial(self, data); }
        public void NotifyPoll(PollMessage.PollType t) { msServerNotifyPoll(self, t); }
    }
    #endregion


    #region Messages
    public enum MessageType
    {
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

    public struct GetMessage
    {
        #region internal
        public IntPtr self;
        [DllImport(Lib.name)] static extern GetFlags msGetGetFlags(IntPtr self);
        [DllImport(Lib.name)] static extern int msGetGetBakeSkin(IntPtr self);
        [DllImport(Lib.name)] static extern int msGetGetBakeCloth(IntPtr self);
        #endregion

        public static explicit operator GetMessage(IntPtr v)
        {
            GetMessage ret;
            ret.self = v;
            return ret;
        }

        internal GetFlags flags { get { return msGetGetFlags(self); } }
        public bool bakeSkin { get { return msGetGetBakeSkin(self) != 0; } }
        public bool bakeCloth { get { return msGetGetBakeCloth(self) != 0; } }
    }

    internal struct SetMessage
    {
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

    internal struct DeleteMessage
    {
        #region internal
        public IntPtr self;
        [DllImport(Lib.name)] static extern int msDeleteGetNumEntities(IntPtr self);
        [DllImport(Lib.name)] static extern Identifier msDeleteGetEntity(IntPtr self, int i);
        [DllImport(Lib.name)] static extern int msDeleteGetNumMaterials(IntPtr self);
        [DllImport(Lib.name)] static extern Identifier msDeleteGetMaterial(IntPtr self, int i);
        #endregion

        public static explicit operator DeleteMessage(IntPtr v)
        {
            DeleteMessage ret;
            ret.self = v;
            return ret;
        }

        public int numEntities { get { return msDeleteGetNumEntities(self); } }
        internal Identifier GetEntity(int i) { return msDeleteGetEntity(self, i); }

        public int numMaterials { get { return msDeleteGetNumMaterials(self); } }
        internal Identifier GetMaterial(int i) { return msDeleteGetMaterial(self, i); }
    }

    public struct FenceMessage
    {
        #region internal
        public IntPtr self;
        [DllImport(Lib.name)] static extern FenceType msFenceGetType(IntPtr self);
        #endregion

        public enum FenceType
        {
            Unknown,
            SceneBegin,
            SceneEnd,
        }

        public static explicit operator FenceMessage(IntPtr v)
        {
            FenceMessage ret;
            ret.self = v;
            return ret;
        }

        public FenceType type { get { return msFenceGetType(self); } }
    }

    public struct TextMessage
    {
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

        public void Print()
        {
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

    public struct QueryMessage
    {
        #region internal
        public IntPtr self;
        [DllImport(Lib.name)] static extern QueryType msQueryGetType(IntPtr self);
        [DllImport(Lib.name)] static extern void msQueryFinishRespond(IntPtr self);
        [DllImport(Lib.name)] static extern void msQueryAddResponseText(IntPtr self, string text);
        #endregion

        public enum QueryType
        {
            Unknown,
            PluginVersion,
            ProtocolVersion,
            HostName,
            RootNodes,
            AllNodes,
        }

        public static explicit operator QueryMessage(IntPtr v)
        {
            QueryMessage ret;
            ret.self = v;
            return ret;
        }

        public QueryType queryType { get { return msQueryGetType(self); } }

        public void FinishRespond()
        {
            msQueryFinishRespond(self);
        }
        public void AddResponseText(string text)
        {
            msQueryAddResponseText(self, text);
        }
    }

    public struct PollMessage
    {
        public enum PollType
        {
            Unknown,
            SceneUpdate,
        }
    }
    #endregion
#endif // UNITY_STANDALONE
}
