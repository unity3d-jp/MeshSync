using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using UnityEngine;
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
    }


    #region Server
    public struct ServerSettings
    {
        public int maxQueue;
        public int maxThreads;
        public ushort port;
        public uint meshSplitUnit;

        public static ServerSettings defaultValue
        {
            get
            {
                return new ServerSettings
                {
                    maxQueue = 512,
                    maxThreads = 8,
                    port = 8080,
#if UNITY_2017_3_OR_NEWER
                    meshSplitUnit = 0xffffffff,
#else
                    meshSplitUnit = 65000,
#endif
                };
            }
        }
    }
    public struct Server
    {
        #region internal
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
    };


    [StructLayout(LayoutKind.Explicit)]
    public struct AssetData
    {
        #region internal
        [FieldOffset(0)] internal IntPtr self;
        [DllImport("MeshSyncServer")] static extern int msAssetGetID(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msAssetSetID(IntPtr self, int v);
        [DllImport("MeshSyncServer")] static extern IntPtr msAssetGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msAssetSetName(IntPtr self, string v);
        [DllImport("MeshSyncServer")] static extern AssetType msAssetGetType(IntPtr self);
        #endregion

        public static implicit operator bool(AssetData v) { return v.self != IntPtr.Zero; }
        public static explicit operator FileAssetData(AssetData v) { return new FileAssetData { self = v.self }; }
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
        [FieldOffset(0)] internal IntPtr self;
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


    #region Material
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

    public struct MaterialPropertyData
    {
        #region internal
        internal IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msMaterialPropGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Type msMaterialPropGetType(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msMaterialPropGetInt(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msMaterialPropGetFloat(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Vector4 msMaterialPropGetVector(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Matrix4x4 msMaterialPropGetMatrix(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msMaterialPropGetTexture(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msMaterialPropGetArraySize(IntPtr self);
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

        public static implicit operator bool(MaterialPropertyData v)
        {
            return v.self != IntPtr.Zero;
        }

        public string name { get { return Misc.S(msMaterialPropGetName(self)); } }
        public Type type { get { return msMaterialPropGetType(self); } }

        public int intValue { get { return msMaterialPropGetInt(self); } }
        public float floatValue { get { return msMaterialPropGetFloat(self); } }
        public Vector4 vectorValue { get { return msMaterialPropGetVector(self); } }
        public Matrix4x4 matrixValue { get { return msMaterialPropGetMatrix(self); } }

        public int arraySize { get { return msMaterialPropGetArraySize(self); } }
        public float[] floatArray
        {
            get
            {
                var ret = new float[arraySize];
                msMaterialPropCopyData(self, ret);
                return ret;
            }
        }
        public Vector4[] vectorArray
        {
            get
            {
                var ret = new Vector4[arraySize];
                msMaterialPropCopyData(self, ret);
                return ret;
            }
        }
        public Matrix4x4[] matrixArray
        {
            get
            {
                var ret = new Matrix4x4[arraySize];
                msMaterialPropCopyData(self, ret);
                return ret;
            }
        }
        public int textureValue { get { return msMaterialPropGetTexture(self); } }
    }

    public struct MaterialKeywordData
    {
        #region internal
        internal IntPtr self;
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
        [FieldOffset(0)] internal IntPtr self;
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
    public enum InterpolationType
    {
        Smooth,
        Linear,
        Constant,
    }
    public delegate void InterpolationMethod(AnimationCurve curve);

    public struct TransformAnimationData
    {
        #region internal
        internal IntPtr self;

        [DllImport("MeshSyncServer")] static extern int msTransformAGetNumTranslationSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msTransformAGetTranslationTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern Vector3 msTransformAGetTranslationValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msTransformAGetNumRotationSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msTransformAGetRotationTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern Quaternion msTransformAGetRotationValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msTransformAGetNumScaleSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msTransformAGetScaleTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern Vector3 msTransformAGetScaleValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msTransformAGetNumVisibleSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msTransformAGetVisibleTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern byte msTransformAGetVisibleValue(IntPtr self, int i);
        #endregion

        public static explicit operator TransformAnimationData(IntPtr v)
        {
            TransformAnimationData ret;
            ret.self = v;
            return ret;
        }
        public static implicit operator bool(TransformAnimationData v)
        {
            return v.self != IntPtr.Zero;
        }

        public AnimationCurve[] GenTranslationCurves()
        {
            int n = msTransformAGetNumTranslationSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            var y = new Keyframe[n];
            var z = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msTransformAGetTranslationTime(self, i);
                var v = msTransformAGetTranslationValue(self, i);
                x[i].time = y[i].time = z[i].time = t;
                x[i].value = v.x;
                y[i].value = v.y;
                z[i].value = v.z;
            }
            var ret = new AnimationCurve[] {
                    new AnimationCurve(x),
                    new AnimationCurve(y),
                    new AnimationCurve(z),
                };
            return ret;
        }

        public AnimationCurve[] GenRotationCurves()
        {
            int n = msTransformAGetNumRotationSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            var y = new Keyframe[n];
            var z = new Keyframe[n];
            var w = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msTransformAGetRotationTime(self, i);
                var v = msTransformAGetRotationValue(self, i);
                x[i].time = y[i].time = z[i].time = w[i].time = t;
                x[i].value = v.x;
                y[i].value = v.y;
                z[i].value = v.z;
                w[i].value = v.w;
            }
            var ret = new AnimationCurve[] {
                    new AnimationCurve(x),
                    new AnimationCurve(y),
                    new AnimationCurve(z),
                    new AnimationCurve(w),
                };
            return ret;
        }

        public AnimationCurve[] GenScaleCurves()
        {
            int n = msTransformAGetNumScaleSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            var y = new Keyframe[n];
            var z = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msTransformAGetScaleTime(self, i);
                var v = msTransformAGetScaleValue(self, i);
                x[i].time = y[i].time = z[i].time = t;
                x[i].value = v.x;
                y[i].value = v.y;
                z[i].value = v.z;
            }
            var ret = new AnimationCurve[] {
                    new AnimationCurve(x),
                    new AnimationCurve(y),
                    new AnimationCurve(z),
                };
            return ret;
        }

        public AnimationCurve GenVisibilityCurve()
        {
            int n = msTransformAGetNumVisibleSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msTransformAGetVisibleTime(self, i);
                var v = msTransformAGetVisibleValue(self, i);
                x[i].time = t;
                x[i].value = v;
            }
            var ret = new AnimationCurve(x);
            return ret;
        }

#if UNITY_EDITOR
        public void ExportToClip(AnimationClip clip, GameObject root, GameObject target, string path, InterpolationMethod im)
        {
            var ttrans = typeof(Transform);
            var tgo = typeof(GameObject);

            {
                clip.SetCurve(path, ttrans, "m_LocalPosition", null);
                var curves = GenTranslationCurves();
                if (curves != null)
                {
                    foreach (var c in curves)
                        im(c);
                    clip.SetCurve(path, ttrans, "m_LocalPosition.x", curves[0]);
                    clip.SetCurve(path, ttrans, "m_LocalPosition.y", curves[1]);
                    clip.SetCurve(path, ttrans, "m_LocalPosition.z", curves[2]);
                }
            }
            {
                clip.SetCurve(path, ttrans, "m_LocalRotation", null);
                var curves = GenRotationCurves();
                if (curves != null)
                {
                    foreach (var c in curves)
                        im(c);
                    clip.SetCurve(path, ttrans, "m_LocalRotation.x", curves[0]);
                    clip.SetCurve(path, ttrans, "m_LocalRotation.y", curves[1]);
                    clip.SetCurve(path, ttrans, "m_LocalRotation.z", curves[2]);
                    clip.SetCurve(path, ttrans, "m_LocalRotation.w", curves[3]);
                }
            }
            {
                clip.SetCurve(path, ttrans, "m_LocalScale", null);
                var curves = GenScaleCurves();
                if (curves != null)
                {
                    foreach (var c in curves)
                        im(c);
                    clip.SetCurve(path, ttrans, "m_LocalScale.x", curves[0]);
                    clip.SetCurve(path, ttrans, "m_LocalScale.y", curves[1]);
                    clip.SetCurve(path, ttrans, "m_LocalScale.z", curves[2]);
                }
            }
            {
                clip.SetCurve(path, tgo, "m_IsActive", null);
                var curve = GenVisibilityCurve();
                if (curve != null)
                    im(curve);
                clip.SetCurve(path, tgo, "m_IsActive", curve);
            }
        }
#endif
    }

    public struct CameraAnimationData
    {
        #region internal
        internal IntPtr self;

        [DllImport("MeshSyncServer")] static extern int msCameraAGetNumFovSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetFovTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetFovValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msCameraAGetNumNearSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetNearTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetNearValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msCameraAGetNumFarSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetFarTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetFarValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msCameraAGetNumHApertureSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetHApertureTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetHApertureValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msCameraAGetNumVApertureSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetVApertureTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetVApertureValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msCameraAGetNumFocalLengthSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetFocalLengthTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetFocalLengthValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msCameraAGetNumFocusDistanceSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetFocusDistanceTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msCameraAGetFocusDistanceValue(IntPtr self, int i);
        #endregion

        public static explicit operator CameraAnimationData(IntPtr v)
        {
            CameraAnimationData ret;
            ret.self = v;
            return ret;
        }
        public static implicit operator bool(CameraAnimationData v)
        {
            return v.self != IntPtr.Zero;
        }

        public AnimationCurve GenFovCurve()
        {
            int n = msCameraAGetNumFovSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msCameraAGetFovTime(self, i);
                var v = msCameraAGetFovValue(self, i);
                x[i].time = t;
                x[i].value = v;
            }
            var ret = new AnimationCurve(x);
            return ret;
        }

        public AnimationCurve GenNearPlaneCurve()
        {
            int n = msCameraAGetNumNearSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msCameraAGetNearTime(self, i);
                var v = msCameraAGetNearValue(self, i);
                x[i].time = t;
                x[i].value = v;
            }
            var ret = new AnimationCurve(x);
            return ret;
        }

        public AnimationCurve GenFarPlaneCurve()
        {
            int n = msCameraAGetNumFarSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msCameraAGetFarTime(self, i);
                var v = msCameraAGetFarValue(self, i);
                x[i].time = t;
                x[i].value = v;
            }
            var ret = new AnimationCurve(x);
            return ret;
        }

#if UNITY_EDITOR
        public void ExportToClip(AnimationClip clip, GameObject root, GameObject target, string path, InterpolationMethod im)
        {
            ((TransformAnimationData)self).ExportToClip(clip, root, target, path, im);

            var tcam = typeof(Camera);
            {
                clip.SetCurve(path, tcam, "field of view", null);
                var curve = GenFovCurve();
                if (curve != null)
                {
                    im(curve);
                    clip.SetCurve(path, tcam, "field of view", curve);
                }
            }
            {
                clip.SetCurve(path, tcam, "near clip plane", null);
                var curve = GenNearPlaneCurve();
                if (curve != null)
                {
                    im(curve);
                    clip.SetCurve(path, tcam, "near clip plane", curve);
                }
            }
            {
                clip.SetCurve(path, tcam, "far clip plane", null);
                var curve = GenFarPlaneCurve();
                if (curve != null)
                {
                    im(curve);
                    clip.SetCurve(path, tcam, "far clip plane", curve);
                }
            }
        }
#endif
    }

    public struct LightAnimationData
    {
        #region internal
        internal IntPtr self;

        [DllImport("MeshSyncServer")] static extern int msLightAGetNumColorSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msLightAGetColorTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern Color msLightAGetColorValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msLightAGetNumIntensitySamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msLightAGetIntensityTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msLightAGetIntensityValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msLightAGetNumRangeSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msLightAGetRangeTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msLightAGetRangeValue(IntPtr self, int i);

        [DllImport("MeshSyncServer")] static extern int msLightAGetNumSpotAngleSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msLightAGetSpotAngleTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msLightAGetSpotAngleValue(IntPtr self, int i);
        #endregion


        public static explicit operator LightAnimationData(IntPtr v)
        {
            LightAnimationData ret;
            ret.self = v;
            return ret;
        }
        public static implicit operator bool(LightAnimationData v)
        {
            return v.self != IntPtr.Zero;
        }

        public AnimationCurve[] GenColorCurves()
        {
            int n = msLightAGetNumColorSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            var y = new Keyframe[n];
            var z = new Keyframe[n];
            var w = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msLightAGetColorTime(self, i);
                var v = msLightAGetColorValue(self, i);
                x[i].time = y[i].time = z[i].time = w[i].time = t;
                x[i].value = v.r;
                y[i].value = v.g;
                z[i].value = v.b;
                w[i].value = v.a;
            }
            var ret = new AnimationCurve[] {
                    new AnimationCurve(x),
                    new AnimationCurve(y),
                    new AnimationCurve(z),
                    new AnimationCurve(w),
                };
            return ret;
        }

        public AnimationCurve GenIntensityCurve()
        {
            int n = msLightAGetNumIntensitySamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msLightAGetIntensityTime(self, i);
                var v = msLightAGetIntensityValue(self, i);
                x[i].time = t;
                x[i].value = v;
            }
            var ret = new AnimationCurve(x);
            return ret;
        }

        public AnimationCurve GenRangeCurve()
        {
            int n = msLightAGetNumRangeSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msLightAGetRangeTime(self, i);
                var v = msLightAGetRangeValue(self, i);
                x[i].time = t;
                x[i].value = v;
            }
            var ret = new AnimationCurve(x);
            return ret;
        }

        public AnimationCurve GenSpotAngleCurve()
        {
            int n = msLightAGetNumSpotAngleSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msLightAGetSpotAngleTime(self, i);
                var v = msLightAGetSpotAngleValue(self, i);
                x[i].time = t;
                x[i].value = v;
            }
            var ret = new AnimationCurve(x);
            return ret;
        }

#if UNITY_EDITOR
        public void ExportToClip(AnimationClip clip, GameObject root, GameObject target, string path, InterpolationMethod im)
        {
            ((TransformAnimationData)self).ExportToClip(clip, root, target, path, im);

            var tlight = typeof(Light);
            {
                clip.SetCurve(path, tlight, "m_Color", null);
                var curves = GenColorCurves();
                if (curves != null)
                {
                    foreach (var c in curves)
                        im(c);
                    clip.SetCurve(path, tlight, "m_Color.r", curves[0]);
                    clip.SetCurve(path, tlight, "m_Color.g", curves[1]);
                    clip.SetCurve(path, tlight, "m_Color.b", curves[2]);
                    clip.SetCurve(path, tlight, "m_Color.a", curves[3]);
                }
            }
            {
                clip.SetCurve(path, tlight, "m_Intensity", null);
                var curve = GenIntensityCurve();
                if (curve != null)
                {
                    im(curve);
                    clip.SetCurve(path, tlight, "m_Intensity", curve);
                }
            }
            {
                clip.SetCurve(path, tlight, "m_Range", null);
                var curve = GenRangeCurve();
                if (curve != null)
                {
                    im(curve);
                    clip.SetCurve(path, tlight, "m_Range", curve);
                }
            }
            {
                clip.SetCurve(path, tlight, "m_SpotAngle", null);
                var curve = GenSpotAngleCurve();
                if (curve != null)
                {
                    im(curve);
                    clip.SetCurve(path, tlight, "m_SpotAngle", curve);
                }
            }
        }
#endif
    }

    public struct MeshAnimationData
    {
        #region internal
        internal IntPtr self;

        [DllImport("MeshSyncServer")] static extern int msMeshAGetNumBlendshapes(IntPtr self);
        [DllImport("MeshSyncServer")] static extern IntPtr msMeshAGetBlendshapeName(IntPtr self, int bi);
        [DllImport("MeshSyncServer")] static extern int msMeshAGetNumBlendshapeSamples(IntPtr self, int bi);
        [DllImport("MeshSyncServer")] static extern float msMeshAGetNumBlendshapeTime(IntPtr self, int bi, int si);
        [DllImport("MeshSyncServer")] static extern float msMeshAGetNumBlendshapeWeight(IntPtr self, int bi, int si);
        #endregion


        public static explicit operator MeshAnimationData(IntPtr v)
        {
            MeshAnimationData ret;
            ret.self = v;
            return ret;
        }
        public static implicit operator bool(MeshAnimationData v)
        {
            return v.self != IntPtr.Zero;
        }

#if UNITY_EDITOR
        public void ExportToClip(AnimationClip clip, GameObject root, GameObject target, string path, InterpolationMethod im)
        {
            ((TransformAnimationData)self).ExportToClip(clip, root, target, path, im);

            var tsmr = typeof(SkinnedMeshRenderer);
            {
                // blendshape animation

                int numBS = msMeshAGetNumBlendshapes(self);
                for (int bi = 0; bi < numBS; ++bi)
                {
                    string name = "blendShape." + Misc.S(msMeshAGetBlendshapeName(self, bi));
                    clip.SetCurve(path, tsmr, name, null);

                    int numKeyframes = msMeshAGetNumBlendshapeSamples(self, bi);
                    if (numKeyframes > 0)
                    {
                        var kf = new Keyframe[numKeyframes];
                        for (int ki = 0; ki < numKeyframes; ++ki)
                        {
                            kf[ki].time = msMeshAGetNumBlendshapeTime(self, bi, ki);
                            kf[ki].value = msMeshAGetNumBlendshapeWeight(self, bi, ki);
                        }

                        var curve = new AnimationCurve(kf);
                        im(curve);
                        clip.SetCurve(path, tsmr, name, curve);
                    }
                }
            }
        }
#endif
    }

    public struct PointsAnimationData
    {
        #region internal
        internal IntPtr self;

        [DllImport("MeshSyncServer")] static extern int msPointsAGetNumTimeSamples(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msPointsAGetTimeTime(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern float msPointsAGetTimeValue(IntPtr self, int i);
        #endregion


        public static explicit operator PointsAnimationData(IntPtr v)
        {
            PointsAnimationData ret;
            ret.self = v;
            return ret;
        }
        public static implicit operator bool(PointsAnimationData v)
        {
            return v.self != IntPtr.Zero;
        }

        public AnimationCurve GenTimeCurve()
        {
            int n = msPointsAGetNumTimeSamples(self);
            if (n == 0)
                return null;
            var x = new Keyframe[n];
            for (int i = 0; i < n; ++i)
            {
                var t = msPointsAGetTimeTime(self, i);
                var v = msPointsAGetTimeValue(self, i);
                x[i].time = t;
                x[i].value = v;
            }
            var ret = new AnimationCurve(x);
            return ret;
        }

#if UNITY_EDITOR
        public void ExportToClip(AnimationClip clip, GameObject root, GameObject target, string path, InterpolationMethod im)
        {
            ((TransformAnimationData)self).ExportToClip(clip, root, target, path, im);

            var tpoints = typeof(Points);
            {
                clip.SetCurve(path, tpoints, "m_time", null);
                var curve = GenTimeCurve();
                if (curve != null)
                {
                    im(curve);
                    clip.SetCurve(path, tpoints, "m_time", curve);
                }
            }
        }
#endif
    }

    public struct AnimationData
    {
        #region internal
        internal IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msAnimationGetPath(IntPtr self);
        [DllImport("MeshSyncServer")] static extern TransformData.Type msAnimationGetType(IntPtr self);
        #endregion


        public static explicit operator AnimationData(IntPtr v)
        {
            AnimationData ret;
            ret.self = v;
            return ret;
        }
        public static implicit operator bool(AnimationData v)
        {
            return v.self != IntPtr.Zero;
        }

        public string path
        {
            get { return Misc.S(msAnimationGetPath(self)); }
        }

        public TransformData.Type type
        {
            get { return msAnimationGetType(self); }
        }

#if UNITY_EDITOR
        public static void SmoothInterpolation(AnimationCurve curve)
        {
            int len = curve.length;
            for (int i = 0; i < len; ++i)
            {
                AnimationUtility.SetKeyLeftTangentMode(curve, i, AnimationUtility.TangentMode.ClampedAuto);
                AnimationUtility.SetKeyRightTangentMode(curve, i, AnimationUtility.TangentMode.ClampedAuto);
            }
        }

        public static void LinearInterpolation(AnimationCurve curve)
        {
            int len = curve.length;
            for (int i = 0; i < len; ++i)
            {
                AnimationUtility.SetKeyLeftTangentMode(curve, i, AnimationUtility.TangentMode.Linear);
                AnimationUtility.SetKeyRightTangentMode(curve, i, AnimationUtility.TangentMode.Linear);
            }
        }

        public static void ConstantInterpolation(AnimationCurve curve)
        {
            int len = curve.length;
            for (int i = 0; i < len; ++i)
            {
                AnimationUtility.SetKeyBroken(curve, i, true);
                AnimationUtility.SetKeyLeftTangentMode(curve, i, AnimationUtility.TangentMode.Constant);
                AnimationUtility.SetKeyRightTangentMode(curve, i, AnimationUtility.TangentMode.Constant);
            }
        }

        public void ExportToClip(AnimationClip clip, GameObject root, GameObject target, string path, InterpolationMethod im)
        {
            switch (type)
            {
                case TransformData.Type.Transform:
                    ((TransformAnimationData)self).ExportToClip(clip, root, target, path, im);
                    break;
                case TransformData.Type.Camera:
                    ((CameraAnimationData)self).ExportToClip(clip, root, target, path, im);
                    break;
                case TransformData.Type.Light:
                    ((LightAnimationData)self).ExportToClip(clip, root, target, path, im);
                    break;
                case TransformData.Type.Mesh:
                    ((MeshAnimationData)self).ExportToClip(clip, root, target, path, im);
                    break;
                case TransformData.Type.Points:
                    ((PointsAnimationData)self).ExportToClip(clip, root, target, path, im);
                    break;
            }
        }
#endif
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct AnimationClipData
    {
        #region internal
        [FieldOffset(0)] internal IntPtr self;
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
        internal IntPtr self;
        [DllImport("MeshSyncServer")] static extern int msIdentifierGetID(IntPtr self);
        [DllImport("MeshSyncServer")] static extern IntPtr msIdentifierGetName(IntPtr self);
        #endregion

        public int id { get { return msIdentifierGetID(self); } }
        public string name { get { return Misc.S(msIdentifierGetName(self)); } }
    }

    public struct TransformData
    {
        #region internal
        internal IntPtr self;
        [DllImport("MeshSyncServer")] static extern TransformData msTransformCreate();
        [DllImport("MeshSyncServer")] static extern Type msTransformGetType(IntPtr self);
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

        public enum Type
        {
            Unknown,
            Transform,
            Camera,
            Light,
            Mesh,
            Points,
        };

        public static explicit operator TransformData(IntPtr v) { return new TransformData { self = v }; }
        public static explicit operator CameraData(TransformData v) { return new CameraData { self = v.self }; }
        public static explicit operator LightData(TransformData v) { return new LightData { self = v.self }; }
        public static explicit operator MeshData(TransformData v) { return new MeshData { self = v.self }; }
        public static explicit operator PointsData(TransformData v) { return new PointsData { self = v.self }; }

        public static TransformData Create()
        {
            return msTransformCreate();
        }

        public Type type
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
        [FieldOffset(0)] internal IntPtr self;
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
        [DllImport("MeshSyncServer")] static extern float msCameraGetHorizontalAperture(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetHorizontalAperture(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern float msCameraGetVerticalAperture(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetVerticalAperture(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern float msCameraGetFocalLength(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetFocalLength(IntPtr self, float v);
        [DllImport("MeshSyncServer")] static extern float msCameraGetFocusDistance(IntPtr self);
        [DllImport("MeshSyncServer")] static extern void msCameraSetFocusDistance(IntPtr self, float v);
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
        public float horizontalAperture
        {
            get { return msCameraGetHorizontalAperture(self); }
            set { msCameraSetHorizontalAperture(self, value); }
        }
        public float verticalAperture
        {
            get { return msCameraGetVerticalAperture(self); }
            set { msCameraSetVerticalAperture(self, value); }
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
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct LightData
    {
        #region internal
        [FieldOffset(0)] internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
        [DllImport("MeshSyncServer")] static extern int msSplitGetNumPoints(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msSplitGetNumIndices(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Vector3 msSplitGetBoundsCenter(IntPtr self);
        [DllImport("MeshSyncServer")] static extern Vector3 msSplitGetBoundsSize(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msSplitGetNumSubmeshes(IntPtr self);
        [DllImport("MeshSyncServer")] static extern SubmeshData msSplitGetSubmesh(IntPtr self, int i);
        #endregion

        public int numPoints { get { return msSplitGetNumPoints(self); } }
        public int numIndices { get { return msSplitGetNumIndices(self); } }
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
        internal IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msBlendShapeGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern float msBlendShapeGetWeight(IntPtr self);
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
        }
        public float weight
        {
            get { return msBlendShapeGetWeight(self); }
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
        public bool hasMaterialIDs
        {
            get { return flags[9]; }
            set { flags[9] = value; }
        }
        public bool hasBones
        {
            get { return flags[10]; }
            set { flags[10] = value; }
        }
        public bool hasBlendshapeWeights
        {
            get { return flags[11]; }
            set { flags[11] = value; }
        }
        public bool hasBlendshapes
        {
            get { return flags[12]; }
            set { flags[12] = value; }
        }
        public bool applyTRS
        {
            get { return flags[13]; }
            set { flags[13] = value; }
        }
    };

    [StructLayout(LayoutKind.Explicit)]
    public struct MeshData
    {
        #region internal
        [FieldOffset(0)] internal IntPtr self;
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
        [DllImport("MeshSyncServer")] static extern void msMeshReadWeights4(IntPtr self, IntPtr dst, SplitData split);
        [DllImport("MeshSyncServer")] static extern void msMeshWriteWeights4(IntPtr self, BoneWeight[] weights, int size);
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
        public void ReadBoneWeights(IntPtr dst, SplitData split) { msMeshReadWeights4(self, dst, split); }
        public void ReadIndices(IntPtr dst, SplitData split) { msMeshReadIndices(self, dst, split); }

        public void WritePoints(Vector3[] v) { msMeshWritePoints(self, v, v.Length); }
        public void WriteNormals(Vector3[] v) { msMeshWriteNormals(self, v, v.Length); }
        public void WriteTangents(Vector4[] v) { msMeshWriteTangents(self, v, v.Length); }
        public void WriteUV0(Vector2[] v) { msMeshWriteUV0(self, v, v.Length); }
        public void WriteUV1(Vector2[] v) { msMeshWriteUV1(self, v, v.Length); }
        public void WriteColors(Color[] v) { msMeshWriteColors(self, v, v.Length); }
        public void WriteWeights(BoneWeight[] v) { msMeshWriteWeights4(self, v, v.Length); }
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
        public bool hasVelocities
        {
            get { return flags[3]; }
            set { flags[3] = value; }
        }
        public bool hasColors
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
        internal IntPtr self;

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
        [FieldOffset(0)] internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
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
        internal IntPtr self;
        [DllImport("MeshSyncServer")] static extern IntPtr msSceneGetName(IntPtr self);
        [DllImport("MeshSyncServer")] static extern int msSceneGetNumObjects(IntPtr self);
        [DllImport("MeshSyncServer")] static extern TransformData msSceneGetObjectData(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern int msSceneGetNumMaterials(IntPtr self);
        [DllImport("MeshSyncServer")] static extern MaterialData msSceneGetMaterialData(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern int msSceneGetNumTextures(IntPtr self);
        [DllImport("MeshSyncServer")] static extern TextureData msSceneGetTextureData(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern int msSceneGetNumConstraints(IntPtr self);
        [DllImport("MeshSyncServer")] static extern ConstraintData msSceneGetConstraintData(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern int msSceneGetNumAnimationClips(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AnimationClipData msSceneGetAnimationClipData(IntPtr self, int i);
        [DllImport("MeshSyncServer")] static extern int msSceneGetNumAssets(IntPtr self);
        [DllImport("MeshSyncServer")] static extern AssetData msSceneGetAssetData(IntPtr self, int i);
        #endregion

        public string name { get { return Misc.S(msSceneGetName(self)); } }
        public int numObjects { get { return msSceneGetNumObjects(self); } }
        public int numMaterials { get { return msSceneGetNumMaterials(self); } }
        public int numTextures { get { return msSceneGetNumTextures(self); } }
        public int numConstraints { get { return msSceneGetNumConstraints(self); } }
        public int numAnimationClips { get { return msSceneGetNumAnimationClips(self); } }
        public int numAssets { get { return msSceneGetNumAssets(self); } }

        public TransformData GetObject(int i) { return msSceneGetObjectData(self, i); }
        public MaterialData GetMaterial(int i) { return msSceneGetMaterialData(self, i); }
        public TextureData GetTexture(int i) { return msSceneGetTextureData(self, i); }
        public ConstraintData GetConstraint(int i) { return msSceneGetConstraintData(self, i); }
        public AnimationClipData GetAnimationClip(int i) { return msSceneGetAnimationClipData(self, i); }
        public AssetData GetAsset(int i) { return msSceneGetAssetData(self, i); }
    }
    #endregion Scene
}
