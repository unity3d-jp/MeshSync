using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using UnityEngine;
#if UNITY_2019_1_OR_NEWER
using Unity.Collections;
#endif
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.MeshSync
{
    public static partial class Misc
    {
        public const int InvalidID = -1;

        public static string S(IntPtr cstring)
        {
            return cstring == IntPtr.Zero ? "" : Marshal.PtrToStringAnsi(cstring);
        }

        public static string SanitizeFileName(string name)
        {
            var reg = new Regex("[:<>|\\*\\?]");
            return reg.Replace(name, "_");
        }

        public static void Resize<T>(List<T> list, int n) where T : new()
        {
            int cur = list.Count;
            if (n < cur)
                list.RemoveRange(n, cur - n);
            else if (n > cur)
            {
                if (n > list.Capacity)
                    list.Capacity = n;
                int a = n - cur;
                for (int i = 0; i < a; ++i)
                    list.Add(new T());
            }
        }

        public static T GetOrAddComponent<T>(GameObject go) where T : Component
        {
            var ret = go.GetComponent<T>();
            if (ret == null)
                ret = go.AddComponent<T>();
            return ret;
        }

        public static UnityEngine.TextureFormat ToUnityTextureFormat(TextureFormat v)
        {
            switch (v)
            {
                case TextureFormat.Ru8: return UnityEngine.TextureFormat.R8;
                case TextureFormat.RGu8: return UnityEngine.TextureFormat.RG16;
                case TextureFormat.RGBu8: return UnityEngine.TextureFormat.RGB24;
                case TextureFormat.RGBAu8: return UnityEngine.TextureFormat.RGBA32;
                case TextureFormat.Rf16: return UnityEngine.TextureFormat.RHalf;
                case TextureFormat.RGf16: return UnityEngine.TextureFormat.RGHalf;
                case TextureFormat.RGBAf16: return UnityEngine.TextureFormat.RGBAHalf;
                case TextureFormat.Rf32: return UnityEngine.TextureFormat.RFloat;
                case TextureFormat.RGf32: return UnityEngine.TextureFormat.RGFloat;
                case TextureFormat.RGBAf32: return UnityEngine.TextureFormat.RGBAFloat;
                default: return UnityEngine.TextureFormat.Alpha8;
            }
        }

        public static TextureFormat ToMSTextureFormat(UnityEngine.TextureFormat v)
        {
            switch (v)
            {
                case UnityEngine.TextureFormat.R8: return TextureFormat.Ru8;
                case UnityEngine.TextureFormat.RG16: return TextureFormat.RGu8;
                case UnityEngine.TextureFormat.RGB24: return TextureFormat.RGBu8;
                case UnityEngine.TextureFormat.RGBA32: return TextureFormat.RGBAu8;
                case UnityEngine.TextureFormat.RHalf: return TextureFormat.Rf16;
                case UnityEngine.TextureFormat.RGHalf: return TextureFormat.RGf16;
                case UnityEngine.TextureFormat.RGBAHalf: return TextureFormat.RGBAf16;
                case UnityEngine.TextureFormat.RFloat: return TextureFormat.Rf32;
                case UnityEngine.TextureFormat.RGFloat: return TextureFormat.RGf32;
                case UnityEngine.TextureFormat.RGBAFloat: return TextureFormat.RGBAf32;
                default: return TextureFormat.Ru8;
            }
        }

#if UNITY_2019_1_OR_NEWER
        // explicit layout doesn't work with generics...

        [StructLayout(LayoutKind.Explicit)]
        struct NAByte
        {
            [FieldOffset(0)] public NativeArray<byte> nativeArray;
            [FieldOffset(0)] public IntPtr pointer;
        }
        public static IntPtr ForceGetPointer(ref NativeArray<byte> na)
        {
            var union = new NAByte();
            union.nativeArray = na;
            return union.pointer;
        }

        [StructLayout(LayoutKind.Explicit)]
        struct NABoneWeight1
        {
            [FieldOffset(0)] public NativeArray<BoneWeight1> nativeArray;
            [FieldOffset(0)] public IntPtr pointer;
        }
        public static IntPtr ForceGetPointer(ref NativeArray<BoneWeight1> na)
        {
            var union = new NABoneWeight1();
            union.nativeArray = na;
            return union.pointer;
        }
#endif
    }


    #region Server
    public struct ServerSettings
    {
        public int maxQueue;
        public int maxThreads;
        public ushort port;
        public uint meshSplitUnit;
        public int meshMaxBoneInfluence; // -1 (variable) or 4

        public static ServerSettings defaultValue
        {
            get
            {
                return new ServerSettings
                {
                    maxQueue = 512,
                    maxThreads = 8,
                    port = defaultPort,
#if UNITY_2017_3_OR_NEWER
                    meshSplitUnit = 0xffffffff,
#else
                    meshSplitUnit = 65000,
#endif
#if UNITY_2019_1_OR_NEWER
                    meshMaxBoneInfluence = -1,
#else
                    meshMaxBoneInfluence = 4,
#endif
                };
            }
        }

        public static ushort defaultPort { get { return 8080; } }
    }
    public struct Server
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msServerGetVersion();
        [DllImport("MeshSyncServer")] static extern Server msServerStart(ref ServerSettings settings);
        [DllImport("MeshSyncServer")] static extern void msServerStop(IntPtr self);

        [DllImport("MeshSyncServer")] static extern int msServerGetNumMessages(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msServerProcessMessages(IntPtr self, MessageHandler handler);

        [DllImport("MeshSyncServer")] static extern void msServerBeginServe(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msServerEndServe(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msServerServeTransform(IntPtr self, TransformData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeCamera(IntPtr self, CameraData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeLight(IntPtr self, LightData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeMesh(IntPtr self, MeshData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeTexture(IntPtr self, TextureData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeMaterial(IntPtr self, MaterialData data);
        [DllImport("MeshSyncServer")] static extern void msServerSetFileRootPath(IntPtr self, string path);
        [DllImport("MeshSyncServer")] static extern void msServerSetScreenshotFilePath(IntPtr self, string path);
        [DllImport("MeshSyncServer")] static extern void msServerNotifyPoll(IntPtr self, PollMessage.PollType t);
        #endregion

        public delegate void MessageHandler(MessageType type, IntPtr data);

        public static implicit operator bool(Server v) { return v.self != IntPtr.Zero; }

        public static string version { get { return Misc.S(msServerGetVersion()); } }

        public static Server Start(ref ServerSettings settings) { return msServerStart(ref settings); }
        public void Stop() { msServerStop(self); }

        public int numMessages { get { return msServerGetNumMessages(self); } }
        public void ProcessMessages(MessageHandler handler) { msServerProcessMessages(self, handler); }

        public string fileRootPath { set { msServerSetFileRootPath(self, value); } }
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

    public struct GetFlags
    {
        public BitFlags flags;
        public bool getTransform { get { return flags[0]; } }
        public bool getPoints { get { return flags[1]; } }
        public bool getNormals { get { return flags[2]; } }
        public bool getTangents { get { return flags[3]; } }
        public bool getUV0 { get { return flags[4]; } }
        public bool getUV1 { get { return flags[5]; } }
        public bool getColors { get { return flags[6]; } }
        public bool getIndices { get { return flags[7]; } }
        public bool getMaterialIDs { get { return flags[8]; } }
        public bool getBones { get { return flags[9]; } }
        public bool getBlendShapes { get { return flags[10]; } }
    }

    public struct GetMessage
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern GetFlags msGetGetFlags(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msGetGetBakeSkin(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msGetGetBakeCloth(IntPtr self);
        #endregion

        public static explicit operator GetMessage(IntPtr v)
        {
            GetMessage ret;
            ret.self = v;
            return ret;
        }

        public GetFlags flags { get { return msGetGetFlags(self); } }
        public bool bakeSkin { get { return msGetGetBakeSkin(self) != 0; } }
        public bool bakeCloth { get { return msGetGetBakeCloth(self) != 0; } }
    }

    public struct SetMessage
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern SceneData msSetGetSceneData(IntPtr self);
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

    public struct DeleteMessage
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern int msDeleteGetNumEntities(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Identifier msDeleteGetEntity(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern int msDeleteGetNumMaterials(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Identifier msDeleteGetMaterial(IntPtr self, int i);
        #endregion

        public static explicit operator DeleteMessage(IntPtr v)
        {
            DeleteMessage ret;
            ret.self = v;
            return ret;
        }

        public int numEntities { get { return msDeleteGetNumEntities(self); } }
        public Identifier GetEntity(int i) { return msDeleteGetEntity(self, i); }

        public int numMaterials { get { return msDeleteGetNumMaterials(self); } }
        public Identifier GetMaterial(int i) { return msDeleteGetMaterial(self, i); }
    }

    public struct FenceMessage
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern FenceType msFenceGetType(IntPtr self);
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
        [DllImport("MeshSyncServer")] static extern IntPtr msTextGetText(IntPtr self);
        [DllImport("MeshSyncServer")] static extern TextType msTextGetType(IntPtr self);
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
        [DllImport("MeshSyncServer")] static extern QueryType msQueryGetType(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msQueryFinishRespond(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msQueryAddResponseText(IntPtr self, string text);
        #endregion

        public enum QueryType
        {
            Unknown,
            ClientName,
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


    #region Asset
    public enum AssetType
    {
        Unknown,
        File,
        Animation,
        Texture,
        Material,
        Audio,
    };


    [StructLayout(LayoutKind.Explicit)]
    public struct AssetData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [DllImport("MeshSyncServer")] static extern int msAssetGetID(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msAssetSetID(IntPtr self, int v);
        [DllImport("MeshSyncServer")] static extern IntPtr msAssetGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msAssetSetName(IntPtr self, string v);
        [DllImport("MeshSyncServer")] static extern AssetType msAssetGetType(IntPtr self);
        #endregion

        public static implicit operator bool(AssetData v) { return v.self != IntPtr.Zero; }
        public static explicit operator FileAssetData(AssetData v) { return new FileAssetData { self = v.self }; }
        public static explicit operator AudioData(AssetData v) { return new AudioData { self = v.self }; }
        public static explicit operator AnimationClipData(AssetData v) { return new AnimationClipData { self = v.self }; }
        public static explicit operator TextureData(AssetData v) { return new TextureData { self = v.self }; }
        public static explicit operator MaterialData(AssetData v) { return new MaterialData { self = v.self }; }

        public int id
        {
            get { return msAssetGetID(self); }
            set { msAssetSetID(self, value); }
        }
        public string name
        {
            get { return Misc.S(msAssetGetName(self)); }
            set { msAssetSetName(self, value); }
        }
        public AssetType type
        {
            get { return msAssetGetType(self); }
        }
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct FileAssetData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [FieldOffset(0)] public AssetData asset;
        [DllImport("MeshSyncServer")] static extern FileAssetData msFileAssetCreate();
        [DllImport("MeshSyncServer")] static extern byte msFileAssetReadFromFile(IntPtr self, string path);
        [DllImport("MeshSyncServer")] static extern byte msFileAssetWriteToFile(IntPtr self, string path);
        #endregion

        public static implicit operator bool(FileAssetData v) { return v.self != IntPtr.Zero; }

        public static FileAssetData Create() { return msFileAssetCreate(); }

        public int id
        {
            get { return asset.id; }
            set { asset.id = value; }
        }
        public string name
        {
            get { return asset.name; }
            set { asset.name = value; }
        }

        public bool ReadFromFile(string path) { return msFileAssetReadFromFile(self, path) != 0; }
        public bool WriteToFile(string path) { return msFileAssetWriteToFile(self, path) != 0; }
    }
    #endregion

    #region Audio
    public enum AudioFormat
    {
        Unknown = 0,
        U8,
        S16,
        S24,
        S32,
        F32,
        RawFile = 100,
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct AudioData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [FieldOffset(0)] public AssetData asset;
        [DllImport("MeshSyncServer")] static extern AudioData msAudioCreate();
        [DllImport("MeshSyncServer")] static extern AudioFormat msAudioGetFormat(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msAudioSetFormat(IntPtr self, AudioFormat v);
        [DllImport("MeshSyncServer")] static extern int msAudioGetFrequency(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msAudioSetFrequency(IntPtr self, int v);
        [DllImport("MeshSyncServer")] static extern int msAudioGetChannels(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msAudioSetChannels(IntPtr self, int v);
        [DllImport("MeshSyncServer")] static extern int msAudioGetSampleLength(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msAudioGetDataAsFloat(IntPtr self, float[] dst);
        [DllImport("MeshSyncServer")] static extern byte msAudioWriteToFile(IntPtr self, string path);
        [DllImport("MeshSyncServer")] static extern byte msAudioExportAsWave(IntPtr self, string path);
        #endregion

        public static implicit operator bool(AudioData v) { return v.self != IntPtr.Zero; }

        public static AudioData Create() { return msAudioCreate(); }

        public int id
        {
            get { return asset.id; }
            set { asset.id = value; }
        }
        public string name
        {
            get { return asset.name; }
            set { asset.name = value; }
        }
        public AudioFormat format
        {
            get { return msAudioGetFormat(self); }
            set { msAudioSetFormat(self, value); }
        }
        public int frequency
        {
            get { return msAudioGetFrequency(self); }
            set { msAudioSetFrequency(self, value); }
        }
        public int channels
        {
            get { return msAudioGetChannels(self); }
            set { msAudioSetChannels(self, value); }
        }
        public int sampleLength
        {
            get { return msAudioGetSampleLength(self); }
        }

        public float[] samples
        {
            get
            {
                var ret = new float[sampleLength];
                msAudioGetDataAsFloat(self, ret);
                return ret;
            }
        }

        public bool WriteToFile(string path)
        {
            return msAudioWriteToFile(self, path) != 0;
        }
        public bool ExportAsWave(string path)
        {
            return msAudioExportAsWave(self, path) != 0;
        }
    }
    #endregion

    #region Texture
    public enum TextureType
    {
        Default,
        NormalMap,
    }

    public enum TextureFormat
    {
        Unknown = 0,

        ChannelMask = 0xF,
        TypeMask = 0xF << 4,
        Type_f16 = 0x1 << 4,
        Type_f32 = 0x2 << 4,
        Type_u8 = 0x3 << 4,
        Type_i16 = 0x4 << 4,
        Type_i32 = 0x5 << 4,

        Rf16 = Type_f16 | 1,
        RGf16 = Type_f16 | 2,
        RGBf16 = Type_f16 | 3,
        RGBAf16 = Type_f16 | 4,
        Rf32 = Type_f32 | 1,
        RGf32 = Type_f32 | 2,
        RGBf32 = Type_f32 | 3,
        RGBAf32 = Type_f32 | 4,
        Ru8 = Type_u8 | 1,
        RGu8 = Type_u8 | 2,
        RGBu8 = Type_u8 | 3,
        RGBAu8 = Type_u8 | 4,
        Ri16 = Type_i16 | 1,
        RGi16 = Type_i16 | 2,
        RGBi16 = Type_i16 | 3,
        RGBAi16 = Type_i16 | 4,
        Ri32 = Type_i32 | 1,
        RGi32 = Type_i32 | 2,
        RGBi32 = Type_i32 | 3,
        RGBAi32 = Type_i32 | 4,

        RawFile = 0x10 << 4,
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct TextureData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [FieldOffset(0)] public AssetData asset;
        [DllImport("MeshSyncServer")] static extern TextureData msTextureCreate();
        [DllImport("MeshSyncServer")] static extern TextureType msTextureGetType(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTextureSetType(IntPtr self, TextureType v);
        [DllImport("MeshSyncServer")] static extern TextureFormat msTextureGetFormat(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTextureSetFormat(IntPtr self, TextureFormat v);
        [DllImport("MeshSyncServer")] static extern int msTextureGetWidth(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTextureSetWidth(IntPtr self, int v);
        [DllImport("MeshSyncServer")] static extern int msTextureGetHeight(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTextureSetHeight(IntPtr self, int v);
        [DllImport("MeshSyncServer")] static extern IntPtr msTextureGetDataPtr(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msTextureGetSizeInByte(IntPtr self);
        [DllImport("MeshSyncServer")] static extern byte msTextureWriteToFile(IntPtr self, string path);
        [DllImport("MeshSyncServer")] static extern byte msWriteToFile(string path, byte[] data, int size);
        #endregion

        public static TextureData Create() { return msTextureCreate(); }

        public int id
        {
            get { return asset.id; }
            set { asset.id = value; }
        }
        public string name
        {
            get { return asset.name; }
            set { asset.name = value; }
        }
        public TextureType type
        {
            get { return msTextureGetType(self); }
            set { msTextureSetType(self, value); }
        }
        public TextureFormat format
        {
            get { return msTextureGetFormat(self); }
            set { msTextureSetFormat(self, value); }
        }
        public int width
        {
            get { return msTextureGetWidth(self); }
            set { msTextureSetWidth(self, value); }
        }
        public int height
        {
            get { return msTextureGetHeight(self); }
            set { msTextureSetHeight(self, value); }
        }
        public int sizeInByte
        {
            get { return msTextureGetSizeInByte(self); }
        }
        public IntPtr dataPtr
        {
            get { return msTextureGetDataPtr(self); }
        }

        public bool WriteToFile(string path)
        {
            return msTextureWriteToFile(self, path) != 0;
        }
        public static bool WriteToFile(string path, byte[] data)
        {
            if (data != null)
                return msWriteToFile(path, data, data.Length) != 0;
            return false;
        }
    }
    #endregion

    #region Material
    public struct MaterialPropertyData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msMaterialPropGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Type msMaterialPropGetType(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msMaterialPropGetArrayLength(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msMaterialPropCopyData(IntPtr self, ref int dst);
        [DllImport("MeshSyncServer")] static extern void msMaterialPropCopyData(IntPtr self, ref float dst);
        [DllImport("MeshSyncServer")] static extern void msMaterialPropCopyData(IntPtr self, ref Vector4 dst);
        [DllImport("MeshSyncServer")] static extern void msMaterialPropCopyData(IntPtr self, ref Matrix4x4 dst);
        [DllImport("MeshSyncServer")] static extern void msMaterialPropCopyData(IntPtr self, ref TextureRecord dst);
        [DllImport("MeshSyncServer")] static extern void msMaterialPropCopyData(IntPtr self, float[] dst);
        [DllImport("MeshSyncServer")] static extern void msMaterialPropCopyData(IntPtr self, Vector4[] dst);
        [DllImport("MeshSyncServer")] static extern void msMaterialPropCopyData(IntPtr self, Matrix4x4[] dst);
        #endregion

        public enum Type
        {
            Unknown,
            Int,
            Float,
            Vector,
            Matrix,
            Texture,
        }

        public struct TextureRecord
        {
            public int id;
            public bool hasScaleOffset;
            public Vector2 scale, offset;
        }

        public static implicit operator bool(MaterialPropertyData v)
        {
            return v.self != IntPtr.Zero;
        }

        public string name { get { return Misc.S(msMaterialPropGetName(self)); } }
        public Type type { get { return msMaterialPropGetType(self); } }

        public int intValue
        {
            get
            {
                int ret = 0;
                msMaterialPropCopyData(self, ref ret);
                return ret;
            }
        }
        public float floatValue
        {
            get
            {
                float ret = 0;
                msMaterialPropCopyData(self, ref ret);
                return ret;
            }
        }
        public Vector4 vectorValue
        {
            get
            {
                Vector4 ret = Vector4.zero;
                msMaterialPropCopyData(self, ref ret);
                return ret;
            }
        }
        public Matrix4x4 matrixValue
        {
            get
            {
                Matrix4x4 ret = Matrix4x4.identity;
                msMaterialPropCopyData(self, ref ret);
                return ret;
            }
        }
        public TextureRecord textureValue
        {
            get
            {
                var ret = default(TextureRecord);
                msMaterialPropCopyData(self, ref ret);
                return ret;
            }
        }

        public int arrayLength { get { return msMaterialPropGetArrayLength(self); } }
        public float[] floatArray
        {
            get
            {
                var ret = new float[arrayLength];
                msMaterialPropCopyData(self, ret);
                return ret;
            }
        }
        public Vector4[] vectorArray
        {
            get
            {
                var ret = new Vector4[arrayLength];
                msMaterialPropCopyData(self, ret);
                return ret;
            }
        }
        public Matrix4x4[] matrixArray
        {
            get
            {
                var ret = new Matrix4x4[arrayLength];
                msMaterialPropCopyData(self, ret);
                return ret;
            }
        }
    }

    public struct MaterialKeywordData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msMaterialKeywordGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern byte msMaterialKeywordGetValue(IntPtr self);
        #endregion

        public static implicit operator bool(MaterialKeywordData v)
        {
            return v.self != IntPtr.Zero;
        }

        public string name { get { return Misc.S(msMaterialKeywordGetName(self)); } }
        public bool value { get { return msMaterialKeywordGetValue(self) != 0; } }
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct MaterialData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [FieldOffset(0)] public AssetData asset;
        [DllImport("MeshSyncServer")] static extern MaterialData msMaterialCreate();
        [DllImport("MeshSyncServer")] static extern int msMaterialGetIndex(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msMaterialSetIndex(IntPtr self, int v);
        [DllImport("MeshSyncServer")] static extern IntPtr msMaterialGetShader(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msMaterialSetShader(IntPtr self, string v);
        [DllImport("MeshSyncServer")] static extern int msMaterialGetNumParams(IntPtr self);
        [DllImport("MeshSyncServer")] static extern MaterialPropertyData msMaterialGetParam(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern MaterialPropertyData msMaterialFindParam(IntPtr self, string name);

        [DllImport("MeshSyncServer")] static extern void msMaterialSetInt(IntPtr self, string name, int v);
        [DllImport("MeshSyncServer")] static extern void msMaterialSetFloat(IntPtr self, string name, float v);
        [DllImport("MeshSyncServer")] static extern void msMaterialSetVector(IntPtr self, string name, Vector4 v);
        [DllImport("MeshSyncServer")] static extern void msMaterialSetMatrix(IntPtr self, string name, Matrix4x4 v);
        [DllImport("MeshSyncServer")] static extern void msMaterialSetFloatArray(IntPtr self, string name, float[] v, int c);
        [DllImport("MeshSyncServer")] static extern void msMaterialSetVectorArray(IntPtr self, string name, Vector4[] v, int c);
        [DllImport("MeshSyncServer")] static extern void msMaterialSetMatrixArray(IntPtr self, string name, Matrix4x4[] v, int c);
        [DllImport("MeshSyncServer")] static extern void msMaterialSetTexture(IntPtr self, string name, TextureData v);

        [DllImport("MeshSyncServer")] static extern int msMaterialGetNumKeywords(IntPtr self);
        [DllImport("MeshSyncServer")] static extern MaterialKeywordData msMaterialGetKeyword(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern void msMaterialAddKeyword(IntPtr self, string name, byte v);
        #endregion

        public static implicit operator bool(MaterialData v)
        {
            return v.self != IntPtr.Zero;
        }

        public static MaterialData Create() { return msMaterialCreate(); }

        public int id
        {
            get { return asset.id; }
            set { asset.id = value; }
        }
        public string name
        {
            get { return asset.name; }
            set { asset.name = value; }
        }
        public int index
        {
            get { return msMaterialGetIndex(self); }
            set { msMaterialSetIndex(self, value); }
        }
        public string shader
        {
            get { return Misc.S(msMaterialGetShader(self)); }
            set { msMaterialSetShader(self, value); }
        }

        public int numProperties
        {
            get { return msMaterialGetNumParams(self); }
        }
        public MaterialPropertyData GetProperty(int i)
        {
            return msMaterialGetParam(self, i);
        }
        public MaterialPropertyData FindProperty(string name)
        {
            return msMaterialFindParam(self, name);
        }

        public Color color
        {
            get
            {
                var p = FindProperty("_Color");
                if (p && p.type == MaterialPropertyData.Type.Vector)
                    return p.vectorValue;
                else
                    return Color.black;
            }
            set
            {
                SetVector("_Color", value);
            }
        }

        public void SetInt(string name, int v) { msMaterialSetInt(self, name, v); }
        public void SetFloat(string name, float v) { msMaterialSetFloat(self, name, v); }
        public void SetVector(string name, Vector4 v) { msMaterialSetVector(self, name, v); }
        public void SetMatrix(string name, Matrix4x4 v) { msMaterialSetMatrix(self, name, v); }
        public void SetFloatArray(string name, float[] v) { msMaterialSetFloatArray(self, name, v, v.Length); }
        public void SetVectorArray(string name, Vector4[] v) { msMaterialSetVectorArray(self, name, v, v.Length); }
        public void SetMatrixArray(string name, Matrix4x4[] v) { msMaterialSetMatrixArray(self, name, v, v.Length); }
        public void SetTexture(string name, TextureData v) { msMaterialSetTexture(self, name, v); }

        public int numKeywords
        {
            get { return msMaterialGetNumKeywords(self); }
        }
        public MaterialKeywordData GetKeyword(int i)
        {
            return msMaterialGetKeyword(self, i);
        }
        public void AddKeyword(string name, bool value)
        {
            msMaterialAddKeyword(self, name, (byte)(value ? 1 : 0));
        }
    }
    #endregion


    #region Animations
    public enum InterpolationMode
    {
        Smooth,
        Linear,
        Constant,
    }


    public class AnimationImportContext
    {
        public AnimationClip clip;
        public GameObject root;
        public GameObject target;
        public string path;
        public InterpolationMode interpolation;
        public bool enableVisibility;
        public Type mainComponentType;
    }

    public struct AnimationCurveData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msCurveGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern DataType msCurveGetDataType(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msCurveGetNumSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern IntPtr msCurveGetBlendshapeName(IntPtr self);

        [DllImport("MeshSyncServer")] static extern byte msCurveFillI(IntPtr self, Keyframe[] x, InterpolationMode im);
        [DllImport("MeshSyncServer")] static extern byte msCurveFillF(IntPtr self, Keyframe[] x, InterpolationMode im);
        [DllImport("MeshSyncServer")] static extern byte msCurveFillF2(IntPtr self, Keyframe[] x, Keyframe[] y, InterpolationMode im);
        [DllImport("MeshSyncServer")] static extern byte msCurveFillF3(IntPtr self, Keyframe[] x, Keyframe[] y, Keyframe[] z, InterpolationMode im);
        [DllImport("MeshSyncServer")] static extern byte msCurveFillF4(IntPtr self, Keyframe[] x, Keyframe[] y, Keyframe[] z, Keyframe[] w, InterpolationMode im);
        [DllImport("MeshSyncServer")] static extern byte msCurveFillQuat(IntPtr self, Keyframe[] x, Keyframe[] y, Keyframe[] z, Keyframe[] w, InterpolationMode im);
        [DllImport("MeshSyncServer")] static extern byte msCurveFillEuler(IntPtr self, Keyframe[] x, Keyframe[] y, Keyframe[] z, InterpolationMode im);
        #endregion

        public enum DataType
        {
            Unknown,
            Int,
            Float,
            Float2,
            Float3,
            Float4,
            Quaternion,
        }

        public static implicit operator bool(AnimationCurveData v)
        {
            return v.self != IntPtr.Zero;
        }

        public string name
        {
            get { return Misc.S(msCurveGetName(self)); }
        }
        public DataType dataType
        {
            get { return msCurveGetDataType(self); }
        }
        public int numSamples
        {
            get { return msCurveGetNumSamples(self); }
        }
        public string blendshapeName
        {
            get { return Misc.S(msCurveGetBlendshapeName(self)); }
        }

        public bool FillI(Keyframe[] x, InterpolationMode im) { return msCurveFillI(self, x, im) != 0; }
        public bool FillF(Keyframe[] x, InterpolationMode im) { return msCurveFillF(self, x, im) != 0; }
        public bool FillF2(Keyframe[] x, Keyframe[] y, InterpolationMode im) { return msCurveFillF2(self, x, y, im) != 0; }
        public bool FillF3(Keyframe[] x, Keyframe[] y, Keyframe[] z, InterpolationMode im) { return msCurveFillF3(self, x, y, z, im) != 0; }
        public bool FillF4(Keyframe[] x, Keyframe[] y, Keyframe[] z, Keyframe[] w, InterpolationMode im) { return msCurveFillF4(self, x, y, z, w, im) != 0; }
        public bool FillQuat(Keyframe[] x, Keyframe[] y, Keyframe[] z, Keyframe[] w, InterpolationMode im) { return msCurveFillQuat(self, x, y, z, w, im) != 0; }
        public bool FillEuler(Keyframe[] x, Keyframe[] y, Keyframe[] z, InterpolationMode im) { return msCurveFillEuler(self, x, y, z, im) != 0; }
    }

    public struct AnimationData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msAnimationGetPath(IntPtr self);
        [DllImport("MeshSyncServer")] static extern EntityType msAnimationGetEntityType(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msAnimationGetNumCurves(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetCurve(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationFindCurve(IntPtr self, string name);

        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetTransformTranslation(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetTransformRotation(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetTransformScale(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetTransformVisible(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetCameraFieldOfView(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetCameraNearPlane(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetCameraFarPlane(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetCameraFocalLength(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetCameraFocusDistance(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetCameraSensorSize(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetCameraLendsShift(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetLightColor(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetLightIntensity(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetLightRange(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetLightSpotAngle(IntPtr self);
        public delegate void msCurveCallback(AnimationCurveData data);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationEachBlendshapeCurves(IntPtr self, msCurveCallback cb);
        [DllImport("MeshSyncServer")] static extern AnimationCurveData msAnimationGetPointsTime(IntPtr self);
        #endregion

        public static implicit operator bool(AnimationData v)
        {
            return v.self != IntPtr.Zero;
        }

        public string path
        {
            get { return Misc.S(msAnimationGetPath(self)); }
        }
        public EntityType entityType
        {
            get { return msAnimationGetEntityType(self); }
        }

        AnimationCurve[] GenCurves(AnimationCurveData data, InterpolationMode im)
        {
            if (!data)
                return null;
            int n = data.numSamples;
            var t = data.dataType;
            if (n == 0 || t == AnimationCurveData.DataType.Unknown)
                return null;

            if (t == AnimationCurveData.DataType.Int)
            {
                var x = new Keyframe[n];
                if (data.FillI(x, im))
                    return new AnimationCurve[] { new AnimationCurve(x) };
            }
            else if (t == AnimationCurveData.DataType.Float)
            {
                var x = new Keyframe[n];
                if (data.FillF(x, im))
                    return new AnimationCurve[] { new AnimationCurve(x) };
            }
            else if (t == AnimationCurveData.DataType.Float2)
            {
                var x = new Keyframe[n];
                var y = new Keyframe[n];
                if (data.FillF2(x, y, im))
                    return new AnimationCurve[] { new AnimationCurve(x), new AnimationCurve(y) };
            }
            else if (t == AnimationCurveData.DataType.Float3)
            {
                var x = new Keyframe[n];
                var y = new Keyframe[n];
                var z = new Keyframe[n];
                if (data.FillF3(x, y, z, im))
                    return new AnimationCurve[] { new AnimationCurve(x), new AnimationCurve(y), new AnimationCurve(z) };
            }
            else if (t == AnimationCurveData.DataType.Float4)
            {
                var x = new Keyframe[n];
                var y = new Keyframe[n];
                var z = new Keyframe[n];
                var w = new Keyframe[n];
                if (data.FillF4(x, y, z, w, im))
                    return new AnimationCurve[] { new AnimationCurve(x), new AnimationCurve(y), new AnimationCurve(z), new AnimationCurve(w) };
            }
            else if (t == AnimationCurveData.DataType.Quaternion)
            {
                if (im == InterpolationMode.Constant || im == InterpolationMode.Linear)
                {
                    var x = new Keyframe[n];
                    var y = new Keyframe[n];
                    var z = new Keyframe[n];
                    if (data.FillEuler(x, y, z, im))
                        return new AnimationCurve[] { new AnimationCurve(x), new AnimationCurve(y), new AnimationCurve(z) };
                }
                else
                {
                    var x = new Keyframe[n];
                    var y = new Keyframe[n];
                    var z = new Keyframe[n];
                    var w = new Keyframe[n];
                    if (data.FillQuat(x, y, z, w, im))
                        return new AnimationCurve[] { new AnimationCurve(x), new AnimationCurve(y), new AnimationCurve(z), new AnimationCurve(w) };
                }
            }
            return null;
        }


#if UNITY_EDITOR
        public void ExportTransformAnimation(AnimationImportContext ctx)
        {
            var clip = ctx.clip;
            var path = ctx.path;
            var interpolation = ctx.interpolation;
            var ttrans = typeof(Transform);

            {
                clip.SetCurve(path, ttrans, "m_LocalPosition", null);
                var curves = GenCurves(msAnimationGetTransformTranslation(self), interpolation);
                if (curves != null && curves.Length == 3)
                {
                    clip.SetCurve(path, ttrans, "m_LocalPosition.x", curves[0]);
                    clip.SetCurve(path, ttrans, "m_LocalPosition.y", curves[1]);
                    clip.SetCurve(path, ttrans, "m_LocalPosition.z", curves[2]);
                }
            }
            {
                clip.SetCurve(path, ttrans, "m_LocalRotation", null);
                var curves = GenCurves(msAnimationGetTransformRotation(self), interpolation);
                if (curves != null)
                {
                    if (curves.Length == 3)
                    {
                        clip.SetCurve(path, ttrans, "m_LocalEulerAnglesHint.x", curves[0]);
                        clip.SetCurve(path, ttrans, "m_LocalEulerAnglesHint.y", curves[1]);
                        clip.SetCurve(path, ttrans, "m_LocalEulerAnglesHint.z", curves[2]);
                    }
                    else if (curves.Length == 4)
                    {
                        clip.SetCurve(path, ttrans, "m_LocalRotation.x", curves[0]);
                        clip.SetCurve(path, ttrans, "m_LocalRotation.y", curves[1]);
                        clip.SetCurve(path, ttrans, "m_LocalRotation.z", curves[2]);
                        clip.SetCurve(path, ttrans, "m_LocalRotation.w", curves[3]);
                    }
                }
            }
            {
                clip.SetCurve(path, ttrans, "m_LocalScale", null);
                var curves = GenCurves(msAnimationGetTransformScale(self), interpolation);
                if (curves != null && curves.Length == 3)
                {
                    clip.SetCurve(path, ttrans, "m_LocalScale.x", curves[0]);
                    clip.SetCurve(path, ttrans, "m_LocalScale.y", curves[1]);
                    clip.SetCurve(path, ttrans, "m_LocalScale.z", curves[2]);
                }
            }
            if (ctx.enableVisibility && ctx.mainComponentType != null)
            {
                const string Target = "m_Enabled";
                clip.SetCurve(path, ctx.mainComponentType, Target, null);
                var curves = GenCurves(msAnimationGetTransformVisible(self), InterpolationMode.Constant);
                if (curves != null && curves.Length == 1)
                    clip.SetCurve(path, ctx.mainComponentType, Target, curves[0]);
            }
        }

        public void ExportCameraAnimation(AnimationImportContext ctx)
        {
            var tcam = typeof(Camera);
            ctx.mainComponentType = tcam;
            ExportTransformAnimation(ctx);

            var clip = ctx.clip;
            var path = ctx.path;
            var interpolation = ctx.interpolation;

            {
                const string Target = "field of view";
                clip.SetCurve(path, tcam, Target, null);
                var curves = GenCurves(msAnimationGetCameraFieldOfView(self), interpolation);
                if (curves != null && curves.Length == 1)
                    clip.SetCurve(path, tcam, Target, curves[0]);
            }
            {
                const string Target = "near clip plane";
                clip.SetCurve(path, tcam, Target, null);
                var curves = GenCurves(msAnimationGetCameraNearPlane(self), interpolation);
                if (curves != null && curves.Length == 1)
                    clip.SetCurve(path, tcam, Target, curves[0]);
            }
            {
                const string Target = "far clip plane";
                clip.SetCurve(path, tcam, Target, null);
                var curves = GenCurves(msAnimationGetCameraFarPlane(self), interpolation);
                if (curves != null && curves.Length == 1)
                    clip.SetCurve(path, tcam, Target, curves[0]);
            }

            // physical camera params
            {
                const string Target = "m_FocalLength";
                clip.SetCurve(path, tcam, Target, null);
                var curves = GenCurves(msAnimationGetCameraFocalLength(self), interpolation);
                if (curves != null && curves.Length == 1)
                    clip.SetCurve(path, tcam, Target, curves[0]);
            }
            {
                clip.SetCurve(path, tcam, "m_SensorSize", null);
                var curves = GenCurves(msAnimationGetCameraSensorSize(self), interpolation);
                if (curves != null && curves.Length == 2)
                {
                    clip.SetCurve(path, tcam, "m_SensorSize.x", curves[0]);
                    clip.SetCurve(path, tcam, "m_SensorSize.y", curves[1]);
                }
            }
            {
                clip.SetCurve(path, tcam, "m_LensShift", null);
                var curves = GenCurves(msAnimationGetCameraLendsShift(self), interpolation);
                if (curves != null && curves.Length == 2)
                {
                    clip.SetCurve(path, tcam, "m_LensShift.x", curves[0]);
                    clip.SetCurve(path, tcam, "m_LensShift.y", curves[1]);
                }
            }
        }

        public void ExportLightAnimation(AnimationImportContext ctx)
        {
            var tlight = typeof(Light);
            ctx.mainComponentType = tlight;
            ExportTransformAnimation(ctx);

            var clip = ctx.clip;
            var path = ctx.path;
            var interpolation = ctx.interpolation;

            {
                clip.SetCurve(path, tlight, "m_Color", null);
                var curves = GenCurves(msAnimationGetLightColor(self), interpolation);
                if (curves != null && curves.Length == 4)
                {
                    clip.SetCurve(path, tlight, "m_Color.r", curves[0]);
                    clip.SetCurve(path, tlight, "m_Color.g", curves[1]);
                    clip.SetCurve(path, tlight, "m_Color.b", curves[2]);
                    clip.SetCurve(path, tlight, "m_Color.a", curves[3]);
                }
            }
            {
                const string Target = "m_Intensity";
                clip.SetCurve(path, tlight, Target, null);
                var curves = GenCurves(msAnimationGetLightIntensity(self), interpolation);
                if (curves != null && curves.Length == 1)
                    clip.SetCurve(path, tlight, Target, curves[0]);
            }
            {
                const string Target = "m_Range";
                clip.SetCurve(path, tlight, Target, null);
                var curves = GenCurves(msAnimationGetLightRange(self), interpolation);
                if (curves != null && curves.Length == 1)
                    clip.SetCurve(path, tlight, Target, curves[0]);
            }
            {
                const string Target = "m_SpotAngle";
                clip.SetCurve(path, tlight, Target, null);
                var curves = GenCurves(msAnimationGetLightSpotAngle(self), interpolation);
                if (curves != null && curves.Length == 1)
                    clip.SetCurve(path, tlight, Target, curves[0]);
            }
        }

        static List<AnimationCurveData> s_blendshapes;
        static void BlendshapeCallback(AnimationCurveData data)
        {
            s_blendshapes.Add(data);
        }

        public void ExportMeshAnimation(AnimationImportContext ctx)
        {
            var tsmr = typeof(SkinnedMeshRenderer);
            ctx.mainComponentType = tsmr;
            ExportTransformAnimation(ctx);

            var clip = ctx.clip;
            var path = ctx.path;
            var interpolation = ctx.interpolation;

            {
                // blendshape animation
                clip.SetCurve(path, tsmr, "blendShape", null);

                s_blendshapes = new List<AnimationCurveData>();
                msAnimationEachBlendshapeCurves(self, BlendshapeCallback);
                int numBS = s_blendshapes.Count;
                for (int bi = 0; bi < numBS; ++bi)
                {
                    var data = s_blendshapes[bi];
                    var curves = GenCurves(data, interpolation);
                    if (curves != null && curves.Length == 1)
                        clip.SetCurve(path, tsmr, "blendShape." + data.blendshapeName, curves[0]);
                }
                s_blendshapes = null;
            }
        }

        public void ExportPointsAnimation(AnimationImportContext ctx)
        {
            var tpoints = typeof(PointCache);
            ctx.mainComponentType = tpoints;
            ExportTransformAnimation(ctx);

            var clip = ctx.clip;
            var path = ctx.path;
            var interpolation = ctx.interpolation;

            {
                const string Target = "m_time";
                clip.SetCurve(path, tpoints, Target, null);
                var curves = GenCurves(msAnimationGetPointsTime(self), interpolation);
                if (curves != null && curves.Length == 1)
                    clip.SetCurve(path, tpoints, Target, curves[0]);
            }
        }

        public void ExportToClip(AnimationImportContext ctx)
        {
            switch (entityType)
            {
                case EntityType.Transform:
                    ExportTransformAnimation(ctx);
                    break;
                case EntityType.Camera:
                    ExportCameraAnimation(ctx);
                    break;
                case EntityType.Light:
                    ExportLightAnimation(ctx);
                    break;
                case EntityType.Mesh:
                    ExportMeshAnimation(ctx);
                    break;
                case EntityType.Points:
                    ExportPointsAnimation(ctx);
                    break;
            }

        }
#endif
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct AnimationClipData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [FieldOffset(0)] public AssetData asset;
        [DllImport("MeshSyncServer")] static extern IntPtr msAssetGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msAnimationClipGetNumAnimations(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationData msAnimationClipGetAnimationData(IntPtr self, int i);
        #endregion

        public int id
        {
            get { return asset.id; }
            set { asset.id = value; }
        }
        public string name
        {
            get { return asset.name; }
            set { asset.name = value; }
        }
        public int numAnimations
        {
            get { return msAnimationClipGetNumAnimations(self); }
        }
        public AnimationData GetAnimation(int i)
        {
            return msAnimationClipGetAnimationData(self, i);
        }
    }
    #endregion


    #region Entities
    public struct Identifier
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern int msIdentifierGetID(IntPtr self);
        [DllImport("MeshSyncServer")] static extern IntPtr msIdentifierGetName(IntPtr self);
        #endregion

        public int id { get { return msIdentifierGetID(self); } }
        public string name { get { return Misc.S(msIdentifierGetName(self)); } }
    }

    public enum EntityType
    {
        Unknown,
        Transform,
        Camera,
        Light,
        Mesh,
        Points,
    };

    public struct TransformData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern TransformData msTransformCreate();
        [DllImport("MeshSyncServer")] static extern EntityType msTransformGetType(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msTransformGetID(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTransformSetID(IntPtr self, int v);
        [DllImport("MeshSyncServer")] static extern int msTransformGetIndex(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTransformSetIndex(IntPtr self, int v);
        [DllImport("MeshSyncServer")] static extern IntPtr msTransformGetPath(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTransformSetPath(IntPtr self, string v);
        [DllImport("MeshSyncServer")] static extern Vector3 msTransformGetPosition(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTransformSetPosition(IntPtr self, Vector3 v);
        [DllImport("MeshSyncServer")] static extern Quaternion msTransformGetRotation(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTransformSetRotation(IntPtr self, Quaternion v);
        [DllImport("MeshSyncServer")] static extern Vector3 msTransformGetScale(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTransformSetScale(IntPtr self, Vector3 v);
        [DllImport("MeshSyncServer")] static extern byte msTransformGetVisible(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTransformSetVisible(IntPtr self, byte v);
        [DllImport("MeshSyncServer")] static extern byte msTransformGetVisibleHierarchy(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTransformSetVisibleHierarchy(IntPtr self, byte v);
        [DllImport("MeshSyncServer")] static extern IntPtr msTransformGetReference(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msTransformSetReference(IntPtr self, string v);
        #endregion

        public static explicit operator TransformData(IntPtr v) { return new TransformData { self = v }; }
        public static explicit operator CameraData(TransformData v) { return new CameraData { self = v.self }; }
        public static explicit operator LightData(TransformData v) { return new LightData { self = v.self }; }
        public static explicit operator MeshData(TransformData v) { return new MeshData { self = v.self }; }
        public static explicit operator PointsData(TransformData v) { return new PointsData { self = v.self }; }

        public static TransformData Create()
        {
            return msTransformCreate();
        }

        public EntityType entityType
        {
            get { return msTransformGetType(self); }
        }
        public int id
        {
            get { return msTransformGetID(self); }
            set { msTransformSetID(self, value); }
        }
        public int index
        {
            get { return msTransformGetIndex(self); }
            set { msTransformSetIndex(self, value); }
        }
        public string path
        {
            get { return Misc.S(msTransformGetPath(self)); }
            set { msTransformSetPath(self, value); }
        }
        public Vector3 position
        {
            get { return msTransformGetPosition(self); }
            set { msTransformSetPosition(self, value); }
        }
        public Quaternion rotation
        {
            get { return msTransformGetRotation(self); }
            set { msTransformSetRotation(self, value); }
        }
        public Vector3 scale
        {
            get { return msTransformGetScale(self); }
            set { msTransformSetScale(self, value); }
        }
        public bool visible
        {
            get { return msTransformGetVisible(self) != 0; }
            set { msTransformSetVisible(self, (byte)(value ? 1 : 0)); }
        }
        public bool visibleHierarchy
        {
            get { return msTransformGetVisibleHierarchy(self) != 0; }
            set { msTransformSetVisibleHierarchy(self, (byte)(value ? 1 : 0)); }
        }
        public string reference
        {
            get { return Misc.S(msTransformGetReference(self)); }
            set { msTransformSetReference(self, value); }
        }
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct CameraData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [FieldOffset(0)] public TransformData transform;
        [DllImport("MeshSyncServer")] static extern CameraData msCameraCreate();
        [DllImport("MeshSyncServer")] static extern byte msCameraIsOrtho(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetOrtho(IntPtr self, byte v);
        [DllImport("MeshSyncServer")] static extern float msCameraGetFov(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetFov(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern float msCameraGetNearPlane(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetNearPlane(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern float msCameraGetFarPlane(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetFarPlane(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern float msCameraGetFocalLength(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetFocalLength(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern float msCameraGetFocusDistance(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetFocusDistance(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern void msCameraGetSensorSize(IntPtr self, ref Vector2 v);
        [DllImport("MeshSyncServer")] static extern void msCameraSetSensorSize(IntPtr self, ref Vector2 v);
        [DllImport("MeshSyncServer")] static extern void msCameraGetLendsShift(IntPtr self, ref Vector2 v);
        [DllImport("MeshSyncServer")] static extern void msCameraSetLendsShift(IntPtr self, ref Vector2 v);
        #endregion


        public static CameraData Create()
        {
            return msCameraCreate();
        }

        public bool orthographic
        {
            get { return msCameraIsOrtho(self) != 0; }
            set { msCameraSetOrtho(self, (byte)(value ? 1 : 0)); }
        }
        public float fov
        {
            get { return msCameraGetFov(self); }
            set { msCameraSetFov(self, value); }
        }
        public float nearClipPlane
        {
            get { return msCameraGetNearPlane(self); }
            set { msCameraSetNearPlane(self, value); }
        }
        public float farClipPlane
        {
            get { return msCameraGetFarPlane(self); }
            set { msCameraSetFarPlane(self, value); }
        }
        public float focalLength
        {
            get { return msCameraGetFocalLength(self); }
            set { msCameraSetFocalLength(self, value); }
        }
        public float focusDistance
        {
            get { return msCameraGetFocusDistance(self); }
            set { msCameraSetFocusDistance(self, value); }
        }
        public Vector2 sensorSize
        {
            get { var v = Vector2.zero; msCameraGetSensorSize(self, ref v); return v; }
            set { msCameraSetSensorSize(self, ref value); }
        }
        public Vector2 lendsShift
        {
            get { var v = Vector2.zero; msCameraGetLendsShift(self, ref v); return v; }
            set { msCameraSetLendsShift(self, ref value); }
        }
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct LightData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [FieldOffset(0)] public TransformData transform;

        [DllImport("MeshSyncServer")] static extern LightData msLightCreate();
        [DllImport("MeshSyncServer")] static extern LightType msLightGetType(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msLightSetType(IntPtr self, LightType v);
        [DllImport("MeshSyncServer")] static extern Color msLightGetColor(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msLightSetColor(IntPtr self, Color v);
        [DllImport("MeshSyncServer")] static extern float msLightGetIntensity(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msLightSetIntensity(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern float msLightGetRange(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msLightSetRange(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern float msLightGetSpotAngle(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msLightSetSpotAngle(IntPtr self, float v);
        #endregion

        public static LightData Create()
        {
            return msLightCreate();
        }
        public LightType type
        {
            get { return msLightGetType(self); }
            set { msLightSetType(self, value); }
        }
        public Color color
        {
            get { return msLightGetColor(self); }
            set { msLightSetColor(self, value); }
        }
        public float intensity
        {
            get { return msLightGetIntensity(self); }
            set { msLightSetIntensity(self, value); }
        }
        public float range
        {
            get { return msLightGetRange(self); }
            set { msLightSetRange(self, value); }
        }
        public float spotAngle
        {
            get { return msLightGetSpotAngle(self); }
            set { msLightSetSpotAngle(self, value); }
        }
    }

    #region Mesh
    public struct SubmeshData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern int msSubmeshGetNumIndices(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msSubmeshReadIndices(IntPtr self, IntPtr dst);
        [DllImport("MeshSyncServer")] static extern int msSubmeshGetMaterialID(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Topology msSubmeshGetTopology(IntPtr self);
        #endregion

        public enum Topology
        {
            Points,
            Lines,
            Triangles,
            Quads,
        };


        public int numIndices { get { return msSubmeshGetNumIndices(self); } }
        public Topology topology { get { return msSubmeshGetTopology(self); } }
        public int materialID { get { return msSubmeshGetMaterialID(self); } }
        public void ReadIndices(PinnedList<int> dst) { msSubmeshReadIndices(self, dst); }
    }

    public struct SplitData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern int msSplitGetNumPoints(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msSplitGetNumIndices(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msSplitGetNumBoneWeights(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Vector3 msSplitGetBoundsCenter(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Vector3 msSplitGetBoundsSize(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msSplitGetNumSubmeshes(IntPtr self);
        [DllImport("MeshSyncServer")] static extern SubmeshData msSplitGetSubmesh(IntPtr self, int i);
        #endregion

        public int numPoints { get { return msSplitGetNumPoints(self); } }
        public int numIndices { get { return msSplitGetNumIndices(self); } }
        public int numBoneWeights { get { return msSplitGetNumBoneWeights(self); } }
        public Bounds bounds { get { return new Bounds(msSplitGetBoundsCenter(self), msSplitGetBoundsSize(self)); } }
        public int numSubmeshes { get { return msSplitGetNumSubmeshes(self); } }

        public SubmeshData GetSubmesh(int i)
        {
            return msSplitGetSubmesh(self, i);
        }
    }

    public struct BlendShapeData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msBlendShapeGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msBlendShapeSetName(IntPtr self, string v);
        [DllImport("MeshSyncServer")] static extern float msBlendShapeGetWeight(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msBlendShapeSetWeight(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern int msBlendShapeGetNumFrames(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msBlendShapeGetFrameWeight(IntPtr self, int f);
        [DllImport("MeshSyncServer")] static extern void msBlendShapeReadPoints(IntPtr self, int f, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msBlendShapeReadNormals(IntPtr self, int f, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msBlendShapeReadTangents(IntPtr self, int f, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msBlendShapeAddFrame(IntPtr self, float weight, int num, Vector3[] v, Vector3[] n, Vector3[] t);
        #endregion

        public string name
        {
            get { return Misc.S(msBlendShapeGetName(self)); }
            set { msBlendShapeSetName(self, value); }
        }
        public float weight
        {
            get { return msBlendShapeGetWeight(self); }
            set { msBlendShapeSetWeight(self, value); }
        }
        public float numFrames
        {
            get { return msBlendShapeGetNumFrames(self); }
        }
        public float GetWeight(int f) { return msBlendShapeGetFrameWeight(self, f); }
        public void ReadPoints(int f, PinnedList<Vector3> dst, SplitData split) { msBlendShapeReadPoints(self, f, dst, split); }
        public void ReadNormals(int f, PinnedList<Vector3> dst, SplitData split) { msBlendShapeReadNormals(self, f, dst, split); }
        public void ReadTangents(int f, PinnedList<Vector3> dst, SplitData split) { msBlendShapeReadTangents(self, f, dst, split); }

        public void AddFrame(float w, Vector3[] v, Vector3[] n, Vector3[] t)
        {
            msBlendShapeAddFrame(self, w, v.Length, v, n, t);
        }
    }

    public struct MeshDataFlags
    {
        public BitFlags flags;
        public bool hasRefineSettings
        {
            get { return flags[0]; }
            set { flags[0] = value; }
        }
        public bool hasIndices
        {
            get { return flags[1]; }
            set { flags[1] = value; }
        }
        public bool hasCounts
        {
            get { return flags[2]; }
            set { flags[2] = value; }
        }
        public bool hasPoints
        {
            get { return flags[3]; }
            set { flags[3] = value; }
        }
        public bool hasNormals
        {
            get { return flags[4]; }
            set { flags[4] = value; }
        }
        public bool hasTangents
        {
            get { return flags[5]; }
            set { flags[5] = value; }
        }
        public bool hasUV0
        {
            get { return flags[6]; }
            set { flags[6] = value; }
        }
        public bool hasUV1
        {
            get { return flags[7]; }
            set { flags[7] = value; }
        }
        public bool hasColors
        {
            get { return flags[8]; }
            set { flags[8] = value; }
        }
        public bool hasVelocities
        {
            get { return flags[9]; }
            set { flags[9] = value; }
        }
        public bool hasMaterialIDs
        {
            get { return flags[10]; }
            set { flags[10] = value; }
        }
        public bool hasBones
        {
            get { return flags[11]; }
            set { flags[11] = value; }
        }
        public bool hasBlendshapeWeights
        {
            get { return flags[12]; }
            set { flags[12] = value; }
        }
        public bool hasBlendshapes
        {
            get { return flags[13]; }
            set { flags[13] = value; }
        }
        public bool applyTRS
        {
            get { return flags[14]; }
            set { flags[14] = value; }
        }
    };

    [StructLayout(LayoutKind.Explicit)]
    public struct MeshData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [FieldOffset(0)] public TransformData transform;

        [DllImport("MeshSyncServer")] static extern MeshData msMeshCreate();
        [DllImport("MeshSyncServer")] static extern MeshDataFlags msMeshGetFlags(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msMeshSetFlags(IntPtr self, MeshDataFlags v);
        [DllImport("MeshSyncServer")] static extern int msMeshGetNumPoints(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msMeshGetNumIndices(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msMeshGetNumSplits(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msMeshReadPoints(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWritePoints(IntPtr self, Vector3[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msMeshReadNormals(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteNormals(IntPtr self, Vector3[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msMeshReadTangents(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteTangents(IntPtr self, Vector4[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msMeshReadUV0(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshReadUV1(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteUV0(IntPtr self, Vector2[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteUV1(IntPtr self, Vector2[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msMeshReadColors(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteColors(IntPtr self, Color[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msMeshReadVelocities(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteVelocities(IntPtr self, Vector3[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msMeshReadBoneWeights4(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteBoneWeights4(IntPtr self, BoneWeight[] weights, int size);
#if UNITY_2019_1_OR_NEWER
        [DllImport("MeshSyncServer")] static extern void msMeshReadBoneCounts(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshReadBoneWeightsV(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteBoneWeightsV(IntPtr self, IntPtr counts, int numCounts, IntPtr weights, int numWeights);
#endif
        [DllImport("MeshSyncServer")] static extern void msMeshReadIndices(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteIndices(IntPtr self, int[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteSubmeshTriangles(IntPtr self, int[] v, int size, int materialID);

        [DllImport("MeshSyncServer")] static extern int msMeshGetNumBones(IntPtr self);
        [DllImport("MeshSyncServer")] static extern IntPtr msMeshGetRootBonePath(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msMeshSetRootBonePath(IntPtr self, string v);
        [DllImport("MeshSyncServer")] static extern IntPtr msMeshGetBonePath(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern void msMeshSetBonePath(IntPtr self, string v, int i);
        [DllImport("MeshSyncServer")] static extern void msMeshReadBindPoses(IntPtr self, Matrix4x4[] v);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteBindPoses(IntPtr self, Matrix4x4[] v, int size);

        [DllImport("MeshSyncServer")] static extern void msMeshSetLocal2World(IntPtr self, ref Matrix4x4 v);
        [DllImport("MeshSyncServer")] static extern void msMeshSetWorld2Local(IntPtr self, ref Matrix4x4 v);

        [DllImport("MeshSyncServer")] static extern SplitData msMeshGetSplit(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern int msMeshGetNumSubmeshes(IntPtr self);
        [DllImport("MeshSyncServer")] static extern SubmeshData msMeshGetSubmesh(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msMeshGetNumBlendShapes(IntPtr self);
        [DllImport("MeshSyncServer")] static extern BlendShapeData msMeshGetBlendShapeData(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern BlendShapeData msMeshAddBlendShape(IntPtr self, string name);
        #endregion

        public static MeshData Create()
        {
            return msMeshCreate();
        }

        public MeshDataFlags flags
        {
            get { return msMeshGetFlags(self); }
            set { msMeshSetFlags(self, value); }
        }

        public int numPoints { get { return msMeshGetNumPoints(self); } }
        public int numIndices { get { return msMeshGetNumIndices(self); } }
        public int numSplits { get { return msMeshGetNumSplits(self); } }

        public void ReadPoints(PinnedList<Vector3> dst, SplitData split) { msMeshReadPoints(self, dst, split); }
        public void ReadNormals(PinnedList<Vector3> dst, SplitData split) { msMeshReadNormals(self, dst, split); }
        public void ReadTangents(PinnedList<Vector4> dst, SplitData split) { msMeshReadTangents(self, dst, split); }
        public void ReadUV0(PinnedList<Vector2> dst, SplitData split) { msMeshReadUV0(self, dst, split); }
        public void ReadUV1(PinnedList<Vector2> dst, SplitData split) { msMeshReadUV1(self, dst, split); }
        public void ReadColors(PinnedList<Color> dst, SplitData split) { msMeshReadColors(self, dst, split); }
        public void ReadVelocities(PinnedList<Vector3> dst, SplitData split) { msMeshReadVelocities(self, dst, split); }
        public void ReadBoneWeights4(IntPtr dst, SplitData split) { msMeshReadBoneWeights4(self, dst, split); }
#if UNITY_2019_1_OR_NEWER
        public void ReadBoneCounts(IntPtr dst, SplitData split) { msMeshReadBoneCounts(self, dst, split); }
        public void ReadBoneWeightsV(IntPtr dst, SplitData split) { msMeshReadBoneWeightsV(self, dst, split); }
#endif
        public void ReadIndices(IntPtr dst, SplitData split) { msMeshReadIndices(self, dst, split); }

        public void WritePoints(Vector3[] v) { msMeshWritePoints(self, v, v.Length); }
        public void WriteNormals(Vector3[] v) { msMeshWriteNormals(self, v, v.Length); }
        public void WriteTangents(Vector4[] v) { msMeshWriteTangents(self, v, v.Length); }
        public void WriteUV0(Vector2[] v) { msMeshWriteUV0(self, v, v.Length); }
        public void WriteUV1(Vector2[] v) { msMeshWriteUV1(self, v, v.Length); }
        public void WriteColors(Color[] v) { msMeshWriteColors(self, v, v.Length); }
        public void WriteVelocities(Vector3[] v) { msMeshWriteVelocities(self, v, v.Length); }
        public void WriteBoneWeights4(BoneWeight[] v) { msMeshWriteBoneWeights4(self, v, v.Length); }
#if UNITY_2019_1_OR_NEWER
        public void WriteBoneWeightsV(ref NativeArray<byte> counts, ref NativeArray<BoneWeight1> weights)
        {
            msMeshWriteBoneWeightsV(self, Misc.ForceGetPointer(ref counts), counts.Length, Misc.ForceGetPointer(ref weights), weights.Length);
        }
#endif
        public void WriteIndices(int[] v) { msMeshWriteIndices(self, v, v.Length); }

        public Matrix4x4 local2world { set { msMeshSetLocal2World(self, ref value); } }
        public Matrix4x4 world2local { set { msMeshSetWorld2Local(self, ref value); } }

        public SplitData GetSplit(int i) { return msMeshGetSplit(self, i); }
        public void WriteSubmeshTriangles(int[] indices, int materialID)
        {
            msMeshWriteSubmeshTriangles(self, indices, indices.Length, materialID);
        }

        public int numBones
        {
            get { return msMeshGetNumBones(self); }
        }
        public string rootBonePath
        {
            get { return Misc.S(msMeshGetRootBonePath(self)); }
            set { msMeshSetRootBonePath(self, value); }
        }
        public Matrix4x4[] bindposes
        {
            get
            {
                var ret = new Matrix4x4[numBones];
                msMeshReadBindPoses(self, ret);
                return ret;
            }
            set { msMeshWriteBindPoses(self, value, value.Length); }
        }
        public void SetBonePaths(MeshSyncServer mss, Transform[] bones)
        {
            int n = bones.Length;
            for (int i = 0; i < n; ++i)
            {
                string path = mss.BuildPath(bones[i]);
                msMeshSetBonePath(self, path, i);
            }
        }
        public string[] GetBonePaths()
        {
            int n = numBones;
            var ret = new string[n];
            for (int i = 0; i < n; ++i)
                ret[i] = Misc.S(msMeshGetBonePath(self, i));
            return ret;
        }

        public int numSubmeshes { get { return msMeshGetNumSubmeshes(self); } }
        public SubmeshData GetSubmesh(int i)
        {
            return msMeshGetSubmesh(self, i);
        }

        public int numBlendShapes { get { return msMeshGetNumBlendShapes(self); } }
        public BlendShapeData GetBlendShapeData(int i)
        {
            return msMeshGetBlendShapeData(self, i);
        }
        public BlendShapeData AddBlendShape(string name)
        {
            return msMeshAddBlendShape(self, name);
        }
    };
    #endregion

    #region Point
    public struct PointsDataFlags
    {
        public BitFlags flags;
        public bool hasPoints
        {
            get { return flags[0]; }
            set { flags[0] = value; }
        }
        public bool hasRotations
        {
            get { return flags[1]; }
            set { flags[1] = value; }
        }
        public bool hasScales
        {
            get { return flags[2]; }
            set { flags[2] = value; }
        }
        public bool hasColors
        {
            get { return flags[3]; }
            set { flags[3] = value; }
        }
        public bool hasVelocities
        {
            get { return flags[4]; }
            set { flags[4] = value; }
        }
        public bool hasIDs
        {
            get { return flags[5]; }
            set { flags[5] = value; }
        }
    };

    public struct PointsCacheData
    {
        #region internal
        public IntPtr self;

        [DllImport("MeshSyncServer")] static extern PointsDataFlags msPointsDataGetFlags(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msPointsDataGetTime(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msPointsDataSetTime(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern void msPointsDataGetBounds(IntPtr self, ref Vector3 center, ref Vector3 extents);
        [DllImport("MeshSyncServer")] static extern int msPointsDataGetNumPoints(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msPointsDataReadPoints(IntPtr self, Vector3[] dst);
        [DllImport("MeshSyncServer")] static extern void msPointsDataWritePoints(IntPtr self, Vector3[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msPointsDataReadRotations(IntPtr self, Quaternion[] dst);
        [DllImport("MeshSyncServer")] static extern void msPointsDataWriteRotations(IntPtr self, Quaternion[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msPointsDataReadScales(IntPtr self, Vector3[] dst);
        [DllImport("MeshSyncServer")] static extern void msPointsDataWriteScales(IntPtr self, Vector3[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msPointsDataReadVelocities(IntPtr self, Vector3[] dst);
        [DllImport("MeshSyncServer")] static extern void msPointsDataWriteVelocities(IntPtr self, Vector3[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msPointsDataReadColors(IntPtr self, Color[] dst);
        [DllImport("MeshSyncServer")] static extern void msPointsDataWriteColors(IntPtr self, Color[] v, int size);
        [DllImport("MeshSyncServer")] static extern void msPointsDataReadIDs(IntPtr self, int[] dst);
        [DllImport("MeshSyncServer")] static extern void msPointsDataWriteIDs(IntPtr self, int[] v, int size);
        #endregion

        public PointsDataFlags flags
        {
            get { return msPointsDataGetFlags(self); }
        }
        public float time
        {
            get { return msPointsDataGetTime(self); }
            set { msPointsDataSetTime(self, value); }
        }
        public Bounds bounds
        {
            get
            {
                Vector3 c = default(Vector3);
                Vector3 e = default(Vector3);
                msPointsDataGetBounds(self, ref c, ref e);
                return new Bounds(c, e);
            }
        }
        public int numPoints { get { return msPointsDataGetNumPoints(self); } }

        public void ReadPoints(Vector3[] dst) { msPointsDataReadPoints(self, dst); }
        public void ReadRotations(Quaternion[] dst) { msPointsDataReadRotations(self, dst); }
        public void ReadScales(Vector3[] dst) { msPointsDataReadScales(self, dst); }
        public void ReadVelocities(Vector3[] dst) { msPointsDataReadVelocities(self, dst); }
        public void ReadColors(Color[] dst) { msPointsDataReadColors(self, dst); }
        public void ReadIDs(int[] dst) { msPointsDataReadIDs(self, dst); }

        public void WritePoints(Vector3[] v) { msPointsDataWritePoints(self, v, v.Length); }
        public void WriteRotations(Quaternion[] v) { msPointsDataWriteRotations(self, v, v.Length); }
        public void WriteScales(Vector3[] v) { msPointsDataWriteScales(self, v, v.Length); }
        public void WriteVelocities(Vector3[] v) { msPointsDataWriteVelocities(self, v, v.Length); }
        public void WriteColors(Color[] v) { msPointsDataWriteColors(self, v, v.Length); }
        public void WriteIDs(int[] v) { msPointsDataWriteIDs(self, v, v.Length); }
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct PointsData
    {
        #region internal
        [FieldOffset(0)] public IntPtr self;
        [FieldOffset(0)] public TransformData transform;

        [DllImport("MeshSyncServer")] static extern PointsData msPointsCreate();
        [DllImport("MeshSyncServer")] static extern int msPointsGetNumData(IntPtr self);
        [DllImport("MeshSyncServer")] static extern PointsCacheData msPointsGetData(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern PointsCacheData msPointsAddData(IntPtr self);
        #endregion

        public static PointsData Create()
        {
            return msPointsCreate();
        }

        public int numData { get { return msPointsGetNumData(self); } }
        public PointsCacheData GetData(int i) { return msPointsGetData(self, i); }
        public PointsCacheData AddData(int i) { return msPointsAddData(self); }
    }
    #endregion
    #endregion


    #region Constraints
    public struct ConstraintData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern ConstraintType msConstraintGetType(IntPtr self);
        [DllImport("MeshSyncServer")] static extern IntPtr msConstraintGetPath(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msConstraintGetNumSources(IntPtr self);
        [DllImport("MeshSyncServer")] static extern IntPtr msConstraintGetSource(IntPtr self, int i);
        #endregion

        public enum ConstraintType
        {
            Unknown,
            Aim,
            Parent,
            Position,
            Rotation,
            Scale,
        }

        public static explicit operator ConstraintData(IntPtr v)
        {
            ConstraintData ret;
            ret.self = v;
            return ret;
        }

        public ConstraintType type { get { return msConstraintGetType(self); } }
        public string path { get { return Misc.S(msConstraintGetPath(self)); } }
        public int numSources { get { return msConstraintGetNumSources(self); } }

        public string GetSourcePath(int i) { return Misc.S(msConstraintGetSource(self, i)); }
    }

    public struct AimConstraintData
    {
        #region internal
        public IntPtr self;
        #endregion


        public static explicit operator AimConstraintData(ConstraintData v)
        {
            AimConstraintData ret;
            ret.self = v.self;
            return ret;
        }
    }

    public struct ParentConstraintData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern Vector3 msParentConstraintGetPositionOffset(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern Quaternion msParentConstraintGetRotationOffset(IntPtr self, int i);
        #endregion


        public static explicit operator ParentConstraintData(ConstraintData v)
        {
            ParentConstraintData ret;
            ret.self = v.self;
            return ret;
        }
        public Vector3 GetPositionOffset(int i) { return msParentConstraintGetPositionOffset(self, i); }
        public Quaternion GetRotationOffset(int i) { return msParentConstraintGetRotationOffset(self, i); }
    }

    public struct PositionConstraintData
    {
        #region internal
        public IntPtr self;
        #endregion

        public static explicit operator PositionConstraintData(ConstraintData v)
        {
            PositionConstraintData ret;
            ret.self = v.self;
            return ret;
        }
    }

    public struct RotationConstraintData
    {
        #region internal
        public IntPtr self;
        #endregion

        public static explicit operator RotationConstraintData(ConstraintData v)
        {
            RotationConstraintData ret;
            ret.self = v.self;
            return ret;
        }
    }

    public struct ScaleConstrainData
    {
        #region internal
        public IntPtr self;
        #endregion

        public static explicit operator ScaleConstrainData(ConstraintData v)
        {
            ScaleConstrainData ret;
            ret.self = v.self;
            return ret;
        }
    }
    #endregion


    #region Scene
    public struct SceneData
    {
        #region internal
        public IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msSceneGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msSceneGetNumAssets(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AssetData msSceneGetAsset(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern int msSceneGetNumEntities(IntPtr self);
        [DllImport("MeshSyncServer")] static extern TransformData msSceneGetEntity(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern int msSceneGetNumConstraints(IntPtr self);
        [DllImport("MeshSyncServer")] static extern ConstraintData msSceneGetConstraint(IntPtr self, int i);
        #endregion

        public string name { get { return Misc.S(msSceneGetName(self)); } }
        public int numAssets { get { return msSceneGetNumAssets(self); } }
        public int numEntities { get { return msSceneGetNumEntities(self); } }
        public int numConstraints { get { return msSceneGetNumConstraints(self); } }

        public AssetData GetAsset(int i) { return msSceneGetAsset(self, i); }
        public TransformData GetEntity(int i) { return msSceneGetEntity(self, i); }
        public ConstraintData GetConstraint(int i) { return msSceneGetConstraint(self, i); }
    }
    #endregion Scene
}
