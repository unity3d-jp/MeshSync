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
                MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
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

        public delegate void MessageHandler(NetworkMessageType type, IntPtr data);

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


#endif // UNITY_STANDALONE
}
