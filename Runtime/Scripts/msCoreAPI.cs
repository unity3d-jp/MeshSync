using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;
using Unity.Collections;

#if AT_USE_SPLINES
using Unity.Mathematics;
#endif

namespace Unity.MeshSync {
internal static class Lib {
    #region internal

    public const string name =
#if UNITY_IOS
            "__internal";
#else
        "mscore";
#endif


    [DllImport(name)]
    private static extern IntPtr msGetPluginVersionStr();

    [DllImport(name)]
    private static extern int msGetProtocolVersion();

    #endregion

    private static string version;

    internal static string GetPluginVersion() {
        if (version == null) {
            IntPtr nativeStr = msGetPluginVersionStr(); //Not direct marshalling because there is no free on C# side.
            version = Marshal.PtrToStringAnsi(nativeStr);
        }

        return version;
    }

    internal static int GetProtocolVersion() {
        return msGetProtocolVersion();
    }

    public const int invalidID = -1;

    public const uint maxVerticesPerMesh =
        0xffffffff;

    public const uint maxBoneInfluence =
        255;
}


#region Asset

internal enum AssetType {
    Unknown,
    File,
    Animation,
    Texture,
    Material,
    Audio
};


[StructLayout(LayoutKind.Explicit)]
internal struct AssetData {
    #region internal

    [FieldOffset(0)] public IntPtr self;

    [DllImport(Lib.name)]
    private static extern int msAssetGetID(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msAssetSetID(IntPtr self, int v);

    [DllImport(Lib.name)]
    private static extern IntPtr msAssetGetName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msAssetSetName(IntPtr self, string v);

    [DllImport(Lib.name)]
    private static extern AssetType msAssetGetType(IntPtr self);

    #endregion

    public static implicit operator bool(AssetData v) {
        return v.self != IntPtr.Zero;
    }

    public static explicit operator FileAssetData(AssetData v) {
        return new FileAssetData { self = v.self };
    }

    public static explicit operator AudioData(AssetData v) {
        return new AudioData { self = v.self };
    }

    public static explicit operator AnimationClipData(AssetData v) {
        return new AnimationClipData { self = v.self };
    }

    public static explicit operator TextureData(AssetData v) {
        return new TextureData { self = v.self };
    }

    public static explicit operator MaterialData(AssetData v) {
        return new MaterialData { self = v.self };
    }

    public int id {
        get { return msAssetGetID(self); }
        set { msAssetSetID(self, value); }
    }

    public string name {
        get { return Misc.S(msAssetGetName(self)); }
        set { msAssetSetName(self, value); }
    }

    public AssetType type {
        get { return msAssetGetType(self); }
    }
}

[StructLayout(LayoutKind.Explicit)]
internal struct FileAssetData {
    #region internal

    [FieldOffset(0)] public IntPtr    self;
    [FieldOffset(0)] public AssetData asset;
    [DllImport(Lib.name)]
    private static extern FileAssetData msFileAssetCreate();
#if UNITY_EDITOR
    [DllImport(Lib.name)]
    private static extern byte msFileAssetReadFromFile(IntPtr self, string path);

    [DllImport(Lib.name)]
    private static extern byte msFileAssetWriteToFile(IntPtr self, string path);
#endif

    #endregion

    public static implicit operator bool(FileAssetData v) {
        return v.self != IntPtr.Zero;
    }

    public static FileAssetData Create() {
        return msFileAssetCreate();
    }

    public int id {
        get { return asset.id; }
        set { asset.id = value; }
    }

    public string name {
        get { return asset.name; }
        set { asset.name = value; }
    }

#if UNITY_EDITOR
    public bool ReadFromFile(string path) {
        return msFileAssetReadFromFile(self, path) != 0;
    }

    public bool WriteToFile(string path) {
        return msFileAssetWriteToFile(self, path) != 0;
    }
#endif
}

#endregion

#region Audio

internal enum AudioFormat {
    Unknown = 0,
    U8,
    S16,
    S24,
    S32,
    F32,
    RawFile = 100
}

/// <summary>
/// AudioData
/// </summary>
[StructLayout(LayoutKind.Explicit)]
internal struct AudioData {
    #region internal

    [FieldOffset(0)] public IntPtr    self;
    [FieldOffset(0)] public AssetData asset;

    [DllImport(Lib.name)]
    private static extern AudioData msAudioCreate();

    [DllImport(Lib.name)]
    private static extern AudioFormat msAudioGetFormat(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msAudioSetFormat(IntPtr self, AudioFormat v);

    [DllImport(Lib.name)]
    private static extern int msAudioGetFrequency(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msAudioSetFrequency(IntPtr self, int v);

    [DllImport(Lib.name)]
    private static extern int msAudioGetChannels(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msAudioSetChannels(IntPtr self, int v);

    [DllImport(Lib.name)]
    private static extern int msAudioGetSampleLength(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msAudioGetDataAsFloat(IntPtr self, float[] dst);
#if UNITY_EDITOR
    [DllImport(Lib.name)]
    private static extern byte msAudioWriteToFile(IntPtr self, string path);

    [DllImport(Lib.name)]
    private static extern byte msAudioExportAsWave(IntPtr self, string path);
#endif

    #endregion

    /// <summary>
    /// Checks if the AudioData has been assigned
    /// </summary>
    /// <param name="v"></param>
    /// <returns>True if assigned, false otherwise</returns>        
    public static implicit operator bool(AudioData v) {
        return v.self != IntPtr.Zero;
    }

    /// <summary>
    /// Creates a new AudioData
    /// </summary>
    /// <returns>The newly created AudioData</returns>
    public static AudioData Create() {
        return msAudioCreate();
    }

    internal int id {
        get { return asset.id; }
        set { asset.id = value; }
    }

    internal string name {
        get { return asset.name; }
        set { asset.name = value; }
    }

    internal AudioFormat format {
        get { return msAudioGetFormat(self); }
        set { msAudioSetFormat(self, value); }
    }

    internal int frequency {
        get { return msAudioGetFrequency(self); }
        set { msAudioSetFrequency(self, value); }
    }

    internal int channels {
        get { return msAudioGetChannels(self); }
        set { msAudioSetChannels(self, value); }
    }

    internal int sampleLength {
        get { return msAudioGetSampleLength(self); }
    }

    internal float[] samples {
        get {
            float[] ret = new float[sampleLength];
            msAudioGetDataAsFloat(self, ret);
            return ret;
        }
    }

#if UNITY_EDITOR
    internal bool WriteToFile(string path) {
        return msAudioWriteToFile(self, path) != 0;
    }

    internal bool ExportAsWave(string path) {
        return msAudioExportAsWave(self, path) != 0;
    }
#endif
}

#endregion

#region Texture

internal enum TextureType {
    Default,
    NormalMap,
    NonColor
}

internal enum TextureFormat {
    Unknown = 0,

    ChannelMask = 0xF,
    TypeMask    = 0xF << 4,
    Type_f16    = 0x1 << 4,
    Type_f32    = 0x2 << 4,
    Type_u8     = 0x3 << 4,
    Type_i16    = 0x4 << 4,
    Type_i32    = 0x5 << 4,

    Rf16    = Type_f16 | 1,
    RGf16   = Type_f16 | 2,
    RGBf16  = Type_f16 | 3,
    RGBAf16 = Type_f16 | 4,
    Rf32    = Type_f32 | 1,
    RGf32   = Type_f32 | 2,
    RGBf32  = Type_f32 | 3,
    RGBAf32 = Type_f32 | 4,
    Ru8     = Type_u8 | 1,
    RGu8    = Type_u8 | 2,
    RGBu8   = Type_u8 | 3,
    RGBAu8  = Type_u8 | 4,
    Ri16    = Type_i16 | 1,
    RGi16   = Type_i16 | 2,
    RGBi16  = Type_i16 | 3,
    RGBAi16 = Type_i16 | 4,
    Ri32    = Type_i32 | 1,
    RGi32   = Type_i32 | 2,
    RGBi32  = Type_i32 | 3,
    RGBAi32 = Type_i32 | 4,

    RawFile = 0x10 << 4
}

/// <summary>
/// TextureData
/// </summary>
[StructLayout(LayoutKind.Explicit)]
internal struct TextureData {
    #region internal

    [FieldOffset(0)] public IntPtr    self;
    [FieldOffset(0)] public AssetData asset;

    [DllImport(Lib.name)]
    private static extern TextureData msTextureCreate();

    [DllImport(Lib.name)]
    private static extern TextureType msTextureGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msTextureSetType(IntPtr self, TextureType v);

    [DllImport(Lib.name)]
    private static extern TextureFormat msTextureGetFormat(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msTextureSetFormat(IntPtr self, TextureFormat v);

    [DllImport(Lib.name)]
    private static extern int msTextureGetWidth(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msTextureSetWidth(IntPtr self, int v);

    [DllImport(Lib.name)]
    private static extern int msTextureGetHeight(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msTextureSetHeight(IntPtr self, int v);

    [DllImport(Lib.name)]
    private static extern IntPtr msTextureGetDataPtr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msTextureGetSizeInByte(IntPtr self);

    [DllImport(Lib.name)]
    private static extern byte msTextureWriteToFile(IntPtr self, string path);

    [DllImport(Lib.name)]
    private static extern byte msWriteToFile(string path, byte[] data, int size);

    #endregion

    /// <summary>
    /// Creates a new TextureData
    /// </summary>
    /// <returns>The newly created TextureData</returns>
    public static TextureData Create() {
        return msTextureCreate();
    }

    internal int id {
        get { return asset.id; }
        set { asset.id = value; }
    }

    internal string name {
        get { return asset.name; }
        set { asset.name = value; }
    }

    internal TextureType type {
        get { return msTextureGetType(self); }
        set { msTextureSetType(self, value); }
    }

    internal TextureFormat format {
        get { return msTextureGetFormat(self); }
        set { msTextureSetFormat(self, value); }
    }

    internal int width {
        get { return msTextureGetWidth(self); }
        set { msTextureSetWidth(self, value); }
    }

    internal int height {
        get { return msTextureGetHeight(self); }
        set { msTextureSetHeight(self, value); }
    }

    internal int sizeInByte {
        get { return msTextureGetSizeInByte(self); }
    }

    internal IntPtr dataPtr {
        get { return msTextureGetDataPtr(self); }
    }

#if UNITY_EDITOR
    internal bool WriteToFile(string path) {
        return msTextureWriteToFile(self, path) != 0;
    }

    internal static bool WriteToFile(string path, byte[] data) {
        if (data != null)
            return msWriteToFile(path, data, data.Length) != 0;
        return false;
    }
#endif
}

#endregion

#region Material

internal struct MaterialPropertyData : IMaterialPropertyData
{
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern IntPtr msMaterialPropGetName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IMaterialPropertyData.Type msMaterialPropGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msMaterialPropGetArrayLength(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msMaterialPropCopyData(IntPtr self, ref int dst);

    [DllImport(Lib.name)]
    private static extern void msMaterialPropCopyData(IntPtr self, ref float dst);

    [DllImport(Lib.name)]
    private static extern void msMaterialPropCopyData(IntPtr self, ref Vector4 dst);

    [DllImport(Lib.name)]
    private static extern void msMaterialPropCopyData(IntPtr self, ref Matrix4x4 dst);

    [DllImport(Lib.name)]
    private static extern void msMaterialPropCopyData(IntPtr self, ref IMaterialPropertyData.TextureRecord dst);

    [DllImport(Lib.name)]
    private static extern void msMaterialPropCopyData(IntPtr self, float[] dst);

    [DllImport(Lib.name)]
    private static extern void msMaterialPropCopyData(IntPtr self, Vector4[] dst);

    [DllImport(Lib.name)]
    private static extern void msMaterialPropCopyData(IntPtr self, Matrix4x4[] dst);

    [DllImport(Lib.name)]
    private static extern void msMaterialPropCopyData(IntPtr self, StringBuilder dst);

    #endregion



    public static implicit operator bool(MaterialPropertyData v) {
        return v.self != IntPtr.Zero;
    }

    public string name {
        get {
            if (self == IntPtr.Zero) return default;

            return Misc.S(msMaterialPropGetName(self));
        }
    }

    public int nameID {
        get { return  Shader.PropertyToID(name); }
    }

    public IMaterialPropertyData.Type type {
        get {
            if (self == IntPtr.Zero) return default;

            return msMaterialPropGetType(self);
        }
    }

    public int intValue {
        get {
            if (self == IntPtr.Zero) return default;

            int ret = 0;
            msMaterialPropCopyData(self, ref ret);
            return ret;
        }
    }

    public float floatValue {
        get {
            if (self == IntPtr.Zero) return default;

            float ret = 0;
            msMaterialPropCopyData(self, ref ret);
            return ret;
        }
    }

    public Vector4 vectorValue {
        get {
            if (self == IntPtr.Zero) return default;

            Vector4 ret = Vector4.zero;
            msMaterialPropCopyData(self, ref ret);
            return ret;
        }
    }

    public Matrix4x4 matrixValue {
        get {
            if (self == IntPtr.Zero) return default;

            Matrix4x4 ret = Matrix4x4.identity;
            msMaterialPropCopyData(self, ref ret);
            return ret;
        }
    }

    public IMaterialPropertyData.TextureRecord textureValue {
        get {
            if (self == IntPtr.Zero) return default;

            IMaterialPropertyData.TextureRecord ret = default(IMaterialPropertyData.TextureRecord);
            msMaterialPropCopyData(self, ref ret);
            return ret;
        }
    }

    public int arrayLength {
        get {
            if (self == IntPtr.Zero) return default;

            return msMaterialPropGetArrayLength(self);
        }
    }

    public float[] floatArray {
        get {
            if (self == IntPtr.Zero) return default;

            float[] ret = new float[arrayLength];
            msMaterialPropCopyData(self, ret);
            return ret;
        }
    }

    public Vector4[] vectorArray {
        get {
            if (self == IntPtr.Zero) return default;

            Vector4[] ret = new Vector4[arrayLength];
            msMaterialPropCopyData(self, ret);
            return ret;
        }
    }

    public Matrix4x4[] matrixArray {
        get {
            if (self == IntPtr.Zero) return default;

            Matrix4x4[] ret = new Matrix4x4[arrayLength];
            msMaterialPropCopyData(self, ret);
            return ret;
        }
    }

    public override string ToString() {
        return name;
    }
}

internal struct MaterialKeywordData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern IntPtr msMaterialKeywordGetName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern byte msMaterialKeywordGetValue(IntPtr self);

    #endregion

    public static implicit operator bool(MaterialKeywordData v) {
        return v.self != IntPtr.Zero;
    }

    public string name {
        get { return Misc.S(msMaterialKeywordGetName(self)); }
    }

    public bool value {
        get { return msMaterialKeywordGetValue(self) != 0; }
    }
}

/// <summary>
/// MaterialData
/// </summary>
[StructLayout(LayoutKind.Explicit)]
internal struct MaterialData {
    #region internal

    [FieldOffset(0)] public IntPtr    self;
    [FieldOffset(0)] public AssetData asset;

    [DllImport(Lib.name)]
    private static extern MaterialData msMaterialCreate();

    [DllImport(Lib.name)]
    private static extern int msMaterialGetIndex(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetIndex(IntPtr self, int v);

    [DllImport(Lib.name)]
    private static extern IntPtr msMaterialGetShader(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetShader(IntPtr self, string v);

    [DllImport(Lib.name)]
    private static extern int msMaterialGetNumParams(IntPtr self);

    [DllImport(Lib.name)]
    private static extern MaterialPropertyData msMaterialGetParam(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern MaterialPropertyData msMaterialFindParam(IntPtr self, string name);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetInt(IntPtr self, string name, int v);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetFloat(IntPtr self, string name, float v);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetVector(IntPtr self, string name, Vector4 v);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetMatrix(IntPtr self, string name, Matrix4x4 v);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetFloatArray(IntPtr self, string name, float[] v, int c);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetVectorArray(IntPtr self, string name, Vector4[] v, int c);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetMatrixArray(IntPtr self, string name, Matrix4x4[] v, int c);

    [DllImport(Lib.name)]
    private static extern void msMaterialSetTexture(IntPtr self, string name, TextureData v);

    [DllImport(Lib.name)]
    private static extern int msMaterialGetNumKeywords(IntPtr self);

    [DllImport(Lib.name)]
    private static extern MaterialKeywordData msMaterialGetKeyword(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern void msMaterialAddKeyword(IntPtr self, string name, byte v);

    #endregion

    /// <summary>
    /// Check if the MaterialData has been assigned 
    /// </summary>
    /// <param name="v"></param>
    /// <returns>True if assigned, false otherwise</returns>
    public static implicit operator bool(MaterialData v) {
        return v.self != IntPtr.Zero;
    }

    /// <summary>
    /// Creates a new MaterialData
    /// </summary>
    /// <returns>The newly created MaterialData</returns>
    public static MaterialData Create() {
        return msMaterialCreate();
    }

    internal int id {
        get { return asset.id; }
        set { asset.id = value; }
    }

    internal string name {
        get { return asset.name; }
        set { asset.name = value; }
    }

    internal int index {
        get { return msMaterialGetIndex(self); }
        set { msMaterialSetIndex(self, value); }
    }

    internal string shader {
        get { return Misc.S(msMaterialGetShader(self)); }
        set { msMaterialSetShader(self, value); }
    }

    internal int numProperties {
        get { return msMaterialGetNumParams(self); }
    }

    internal MaterialPropertyData GetProperty(int i) {
        return msMaterialGetParam(self, i);
    }

    internal MaterialPropertyData FindProperty(string name) {
        return msMaterialFindParam(self, name);
    }

    internal Color color {
        get {
            MaterialPropertyData p = FindProperty("_Color");
            if (p && p.type == IMaterialPropertyData.Type.Vector)
                return p.vectorValue;
            else
                return Color.white;
        }
        set { SetVector("_Color", value); }
    }

    internal void SetInt(string name, int v) {
        msMaterialSetInt(self, name, v);
    }

    internal void SetFloat(string name, float v) {
        msMaterialSetFloat(self, name, v);
    }

    internal void SetVector(string name, Vector4 v) {
        msMaterialSetVector(self, name, v);
    }

    internal void SetMatrix(string name, Matrix4x4 v) {
        msMaterialSetMatrix(self, name, v);
    }

    internal void SetFloatArray(string name, float[] v) {
        msMaterialSetFloatArray(self, name, v, v.Length);
    }

    internal void SetVectorArray(string name, Vector4[] v) {
        msMaterialSetVectorArray(self, name, v, v.Length);
    }

    internal void SetMatrixArray(string name, Matrix4x4[] v) {
        msMaterialSetMatrixArray(self, name, v, v.Length);
    }

    internal void SetTexture(string name, TextureData v) {
        msMaterialSetTexture(self, name, v);
    }

    internal int numKeywords {
        get { return msMaterialGetNumKeywords(self); }
    }

    internal MaterialKeywordData GetKeyword(int i) {
        return msMaterialGetKeyword(self, i);
    }

    internal void AddKeyword(string name, bool value) {
        msMaterialAddKeyword(self, name, (byte)(value ? 1 : 0));
    }
}

#endregion


#region Animations

internal struct TimeRange {
    internal float GetDuration() {
        return end - start;
    }

    public float start, end;
}

internal enum InterpolationMode {
    Smooth,
    Linear,
    Constant
}

internal class AnimationImportContext {
    public AnimationClip clip;
    public Type          mainComponentType;
    public GameObject    root;
    public GameObject    target;
    public string        path;
    public bool          enableVisibility;
    public bool          usePhysicalCameraParams;
}

internal struct AnimationCurveData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern IntPtr msCurveGetName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern DataType msCurveGetDataType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msCurveGetNumSamples(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msCurveGetBlendshapeName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msCurveGetNumElements(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msCurveGetNumKeys(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern void msCurveCopy(IntPtr self, int i, Keyframe[] x);

    [DllImport(Lib.name)]
    private static extern void msCurveConvert(IntPtr self, InterpolationMode it);

    #endregion

    public enum DataType {
        Unknown,
        Int,
        Float,
        Float2,
        Float3,
        Float4,
        Quaternion
    }

    public static implicit operator bool(AnimationCurveData v) {
        return v.self != IntPtr.Zero;
    }

    public string name {
        get { return Misc.S(msCurveGetName(self)); }
    }

    public DataType dataType {
        get { return msCurveGetDataType(self); }
    }

    public int numSamples {
        get { return msCurveGetNumSamples(self); }
    }

    public string blendshapeName {
        get { return Misc.S(msCurveGetBlendshapeName(self)); }
    }

    public int numElements {
        get { return msCurveGetNumElements(self); }
    }

    public int GetKeyCount(int i) {
        return msCurveGetNumKeys(self, i);
    }

    public void Copy(int i, Keyframe[] dst) {
        msCurveCopy(self, i, dst);
    }

    public void Convert(InterpolationMode it) {
        msCurveConvert(self, it);
    }
}

/// <summary>
/// AnimationData
/// </summary>
internal struct AnimationData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern IntPtr msAnimationGetPath(IntPtr self);

    [DllImport(Lib.name)]
    private static extern EntityType msAnimationGetEntityType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msAnimationGetNumCurves(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetCurve(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationFindCurve(IntPtr self, string name);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetTransformTranslation(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetTransformRotation(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetTransformScale(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetTransformVisible(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetCameraFieldOfView(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetCameraNearPlane(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetCameraFarPlane(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetCameraFocalLength(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetCameraSensorSize(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetCameraLensShift(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetLightColor(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetLightIntensity(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetLightRange(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationGetLightSpotAngle(IntPtr self);

    internal delegate void msCurveCallback(AnimationCurveData data);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msAnimationEachBlendshapeCurves(IntPtr self, msCurveCallback cb);

    #endregion

    /// <summary>
    /// Checks if the AnimationData has been assigned
    /// </summary>
    /// <param name="v"></param>
    /// <returns>True if assigned, false otherwise</returns>
    public static implicit operator bool(AnimationData v) {
        return v.self != IntPtr.Zero;
    }

    internal string path {
        get { return Misc.S(msAnimationGetPath(self)); }
    }

    internal EntityType entityType {
        get { return msAnimationGetEntityType(self); }
    }

    private AnimationCurve[] GenCurves(AnimationCurveData data) {
        if (!data)
            return null;

        int              numElements = data.numElements;
        AnimationCurve[] ret         = new AnimationCurve[numElements];
        for (int i = 0; i < numElements; ++i) {
            int keyCount = data.GetKeyCount(i);
            if (keyCount > 0) {
                Keyframe[] keys = new Keyframe[keyCount];
                data.Copy(i, keys);
                ret[i] = new AnimationCurve(keys);
            }
        }

        return ret;
    }


#if UNITY_EDITOR
    internal static void SetCurve(AnimationClip clip, string path, Type type, string prop, AnimationCurve curve,
        bool applyNullCurve = false) {
        if (curve == null && !applyNullCurve)
            return;
        Misc.SetCurve(clip, path, type, prop, curve);
    }

    internal void ExportTransformAnimation(AnimationImportContext ctx) {
        AnimationClip clip   = ctx.clip;
        string        path   = ctx.path;
        Type          ttrans = typeof(Transform);

        {
            SetCurve(clip, path, ttrans, "m_LocalPosition", null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetTransformTranslation(self));
            if (curves != null && curves.Length == 3) {
                SetCurve(clip, path, ttrans, "m_LocalPosition.x", curves[0]);
                SetCurve(clip, path, ttrans, "m_LocalPosition.y", curves[1]);
                SetCurve(clip, path, ttrans, "m_LocalPosition.z", curves[2]);
            }
        }
        {
            SetCurve(clip, path, ttrans, "m_LocalEuler", null, true);
            SetCurve(clip, path, ttrans, "m_LocalRotation", null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetTransformRotation(self));
            if (curves != null) {
                if (curves.Length == 3) {
                    SetCurve(clip, path, ttrans, "m_LocalEulerAnglesHint.x", curves[0]);
                    SetCurve(clip, path, ttrans, "m_LocalEulerAnglesHint.y", curves[1]);
                    SetCurve(clip, path, ttrans, "m_LocalEulerAnglesHint.z", curves[2]);
                }
                else if (curves.Length == 4) {
                    SetCurve(clip, path, ttrans, "m_LocalRotation.x", curves[0]);
                    SetCurve(clip, path, ttrans, "m_LocalRotation.y", curves[1]);
                    SetCurve(clip, path, ttrans, "m_LocalRotation.z", curves[2]);
                    SetCurve(clip, path, ttrans, "m_LocalRotation.w", curves[3]);
                }
            }
        }
        {
            SetCurve(clip, path, ttrans, "m_LocalScale", null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetTransformScale(self));
            if (curves != null && curves.Length == 3) {
                SetCurve(clip, path, ttrans, "m_LocalScale.x", curves[0]);
                SetCurve(clip, path, ttrans, "m_LocalScale.y", curves[1]);
                SetCurve(clip, path, ttrans, "m_LocalScale.z", curves[2]);
            }
        }
        if (ctx.enableVisibility && ctx.mainComponentType != null) {
            const string Target = "m_Enabled";
            SetCurve(clip, path, ctx.mainComponentType, Target, null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetTransformVisible(self));
            if (curves != null && curves.Length == 1)
                SetCurve(clip, path, ctx.mainComponentType, Target, curves[0]);
        }
    }

    internal void ExportCameraAnimation(AnimationImportContext ctx) {
        Type tcam = typeof(Camera);
        ctx.mainComponentType = tcam;
        ExportTransformAnimation(ctx);

        AnimationClip clip = ctx.clip;
        string        path = ctx.path;

        // use physical camera params if available
        bool isPhysicalCameraParamsAvailable = false;
        if (ctx.usePhysicalCameraParams) {
            const string Target = "m_FocalLength";
            SetCurve(clip, path, tcam, Target, null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetCameraFocalLength(self));
            if (curves != null && curves.Length == 1) {
                SetCurve(clip, path, tcam, Target, curves[0]);
                isPhysicalCameraParamsAvailable = true;
            }
        }

        if (isPhysicalCameraParamsAvailable) {
            {
                SetCurve(clip, path, tcam, "m_SensorSize", null, true);
                AnimationCurve[] curves = GenCurves(msAnimationGetCameraSensorSize(self));
                if (curves != null && curves.Length == 2) {
                    SetCurve(clip, path, tcam, "m_SensorSize.x", curves[0]);
                    SetCurve(clip, path, tcam, "m_SensorSize.y", curves[1]);
                }
            }
            {
                SetCurve(clip, path, tcam, "m_LensShift", null, true);
                AnimationCurve[] curves = GenCurves(msAnimationGetCameraLensShift(self));
                if (curves != null && curves.Length == 2) {
                    SetCurve(clip, path, tcam, "m_LensShift.x", curves[0]);
                    SetCurve(clip, path, tcam, "m_LensShift.y", curves[1]);
                }
            }
        }
        else {
            const string Target = "field of view";
            SetCurve(clip, path, tcam, Target, null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetCameraFieldOfView(self));
            if (curves != null && curves.Length == 1)
                SetCurve(clip, path, tcam, Target, curves[0]);
        }

        {
            const string Target = "far clip plane";
            SetCurve(clip, path, tcam, Target, null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetCameraFarPlane(self));
            if (curves != null && curves.Length == 1)
                SetCurve(clip, path, tcam, Target, curves[0]);
        }
        {
            const string Target = "near clip plane";
            SetCurve(clip, path, tcam, Target, null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetCameraNearPlane(self));
            if (curves != null && curves.Length == 1)
                SetCurve(clip, path, tcam, Target, curves[0]);
        }
    }

    internal void ExportLightAnimation(AnimationImportContext ctx) {
        Type tlight = typeof(Light);
        ctx.mainComponentType = tlight;
        ExportTransformAnimation(ctx);

        AnimationClip clip = ctx.clip;
        string        path = ctx.path;

        {
            SetCurve(clip, path, tlight, "m_Color", null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetLightColor(self));
            if (curves != null && curves.Length == 4) {
                SetCurve(clip, path, tlight, "m_Color.r", curves[0]);
                SetCurve(clip, path, tlight, "m_Color.g", curves[1]);
                SetCurve(clip, path, tlight, "m_Color.b", curves[2]);
                SetCurve(clip, path, tlight, "m_Color.a", curves[3]);
            }
        }
        {
            const string Target = "m_Intensity";
            SetCurve(clip, path, tlight, Target, null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetLightIntensity(self));
            if (curves != null && curves.Length == 1)
                SetCurve(clip, path, tlight, Target, curves[0]);
        }
        {
            const string Target = "m_Range";
            SetCurve(clip, path, tlight, Target, null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetLightRange(self));
            if (curves != null && curves.Length == 1)
                SetCurve(clip, path, tlight, Target, curves[0]);
        }
        {
            const string Target = "m_SpotAngle";
            SetCurve(clip, path, tlight, Target, null, true);
            AnimationCurve[] curves = GenCurves(msAnimationGetLightSpotAngle(self));
            if (curves != null && curves.Length == 1)
                SetCurve(clip, path, tlight, Target, curves[0]);
        }
    }

    private static List<AnimationCurveData> s_blendshapes;

    private static void BlendshapeCallback(AnimationCurveData data) {
        s_blendshapes.Add(data);
    }

    internal void ExportMeshAnimation(AnimationImportContext ctx) {
        if (ctx.mainComponentType == null)
            ctx.mainComponentType = typeof(MeshRenderer);
        Type tsmr = typeof(SkinnedMeshRenderer);
        ExportTransformAnimation(ctx);

        AnimationClip clip = ctx.clip;
        string        path = ctx.path;

        {
            // blendshape animation
            SetCurve(clip, path, tsmr, "blendShape", null, true);

            s_blendshapes = new List<AnimationCurveData>();
            msAnimationEachBlendshapeCurves(self, BlendshapeCallback);
            int numBS = s_blendshapes.Count;
            for (int bi = 0; bi < numBS; ++bi) {
                AnimationCurveData data   = s_blendshapes[bi];
                AnimationCurve[]   curves = GenCurves(data);
                if (curves != null && curves.Length == 1)
                    SetCurve(clip, path, tsmr, "blendShape." + data.blendshapeName, curves[0]);
            }

            s_blendshapes = null;
        }
    }

    internal void ExportPointsAnimation(AnimationImportContext ctx) {
        Type tpoints = typeof(PointCache);
        ctx.mainComponentType = tpoints;
        ExportTransformAnimation(ctx);
    }

    internal void ExportToClip(AnimationImportContext ctx) {
        switch (entityType) {
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

/// <summary>
/// AnimationClipData
/// </summary>
[StructLayout(LayoutKind.Explicit)]
internal struct AnimationClipData {
    #region internal

    [FieldOffset(0)] public IntPtr    self;
    [FieldOffset(0)] public AssetData asset;

    [DllImport(Lib.name)]
    private static extern IntPtr msAssetGetName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern float msAnimationClipGetFrameRate(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msAnimationClipGetNumAnimations(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationData msAnimationClipGetAnimationData(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern void msAnimationClipConvert(IntPtr self, InterpolationMode it);

    [DllImport(Lib.name)]
    private static extern void msAnimationClipKeyframeReduction(IntPtr self, float threshold, byte eraseFlatCurves);

    [DllImport(Lib.name)]
    private static extern void msSetSizeOfKeyframe(int v);

    private static bool s_prepared = false;

    #endregion

    internal int id {
        get { return asset.id; }
        set { asset.id = value; }
    }

    internal string name {
        get { return asset.name; }
        set { asset.name = value; }
    }

    internal float frameRate {
        get { return msAnimationClipGetFrameRate(self); }
    }

    internal int numAnimations {
        get { return msAnimationClipGetNumAnimations(self); }
    }

    internal AnimationData GetAnimation(int i) {
        return msAnimationClipGetAnimationData(self, i);
    }

    internal static void Prepare() {
        if (!s_prepared) {
            s_prepared = true;
            msSetSizeOfKeyframe(Marshal.SizeOf(typeof(Keyframe)));
        }
    }

    internal void Convert(InterpolationMode it) {
        Prepare();
        msAnimationClipConvert(self, it);
    }

    internal void KeyframeReduction(float threshold, bool eraseFlatCurves) {
        msAnimationClipKeyframeReduction(self, threshold, Misc.ToByte(eraseFlatCurves));
    }
}

#endregion


#region Variant

internal struct VariantData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern IntPtr msVariantGetName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Type msVariantGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msVariantGetArrayLength(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, ref int dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, ref float dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, ref Vector2 dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, ref Vector3 dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, ref Vector4 dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, ref Matrix4x4 dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, int[] dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, float[] dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, Vector2[] dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, Vector3[] dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, Vector4[] dst);

    [DllImport(Lib.name)]
    private static extern void msVariantCopyData(IntPtr self, Matrix4x4[] dst);

    #endregion

    public static explicit operator bool(VariantData v) {
        return v.self != IntPtr.Zero;
    }

    public enum Type {
        Unknown,
        String,
        Int,
        Float,
        Float2,
        Float3,
        Float4,
        Quat,
        Float2x2,
        Float3x3,
        Float4x4
    }

    internal string name {
        get { return Misc.S(msVariantGetName(self)); }
    }

    internal Type type {
        get { return msVariantGetType(self); }
    }

    internal int arrayLength {
        get { return msVariantGetArrayLength(self); }
    }

    public int intValue {
        get {
            int ret = 0;
            msVariantCopyData(self, ref ret);
            return ret;
        }
    }

    public float floatValue {
        get {
            float ret = 0;
            msVariantCopyData(self, ref ret);
            return ret;
        }
    }

    public Vector2 vector2Value {
        get {
            Vector2 ret = Vector2.zero;
            msVariantCopyData(self, ref ret);
            return ret;
        }
    }

    public Vector3 vector3Value {
        get {
            Vector3 ret = Vector3.zero;
            msVariantCopyData(self, ref ret);
            return ret;
        }
    }

    public Vector4 vector4Value {
        get {
            Vector4 ret = Vector4.zero;
            msVariantCopyData(self, ref ret);
            return ret;
        }
    }

    public Matrix4x4 matrixValue {
        get {
            Matrix4x4 ret = Matrix4x4.identity;
            msVariantCopyData(self, ref ret);
            return ret;
        }
    }

    public float[] floatArray {
        get {
            float[] ret = new float[arrayLength];
            msVariantCopyData(self, ret);
            return ret;
        }
    }

    public Vector2[] vector2Array {
        get {
            Vector2[] ret = new Vector2[arrayLength];
            msVariantCopyData(self, ret);
            return ret;
        }
    }

    public Vector3[] vector3Array {
        get {
            Vector3[] ret = new Vector3[arrayLength];
            msVariantCopyData(self, ret);
            return ret;
        }
    }

    public Vector4[] vector4Array {
        get {
            Vector4[] ret = new Vector4[arrayLength];
            msVariantCopyData(self, ret);
            return ret;
        }
    }

    public Matrix4x4[] matrixArray {
        get {
            Matrix4x4[] ret = new Matrix4x4[arrayLength];
            msVariantCopyData(self, ret);
            return ret;
        }
    }
}

#endregion

#region Entities

internal struct Identifier {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern int msIdentifierGetID(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msIdentifierGetName(IntPtr self);

    #endregion

    public int id {
        get { return msIdentifierGetID(self); }
    }

    public string name {
        get { return Misc.S(msIdentifierGetName(self)); }
    }
}

internal enum EntityType {
    Unknown,
    Transform,
    Camera,
    Light,
    Mesh,
    Points,
    Curve
};

internal struct TransformDataFlags {
    public BitFlags flags;

    public bool unchanged {
        get { return flags[0]; }
    }

    public bool hasPosition {
        get { return flags[1]; }
    }

    public bool hasRotation {
        get { return flags[2]; }
    }

    public bool hasScale {
        get { return flags[3]; }
    }

    public bool hasVisibility {
        get { return flags[4]; }
    }

    public bool hasReference {
        get { return flags[7]; }
    }
}

[Serializable]
internal struct VisibilityFlags {
    public BitFlags flags;

    public bool active {
        get { return flags[0]; }
    }

    public bool visibleInRender {
        get { return flags[1]; }
    }

    public bool visibleInViewport {
        get { return flags[2]; }
    }

    public bool castShadows {
        get { return flags[3]; }
    }

    public bool ReceiveShadows {
        get { return flags[4]; }
    }
}

/// <summary>
/// TransformData
/// </summary>
internal struct TransformData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern TransformData msTransformCreate();

    [DllImport(Lib.name)]
    private static extern TransformDataFlags msTransformGetDataFlags(IntPtr self);

    [DllImport(Lib.name)]
    private static extern EntityType msTransformGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msTransformGetHostID(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msTransformGetIndex(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msTransformGetPath(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Vector3 msTransformGetPosition(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Quaternion msTransformGetRotation(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Vector3 msTransformGetScale(IntPtr self);

    [DllImport(Lib.name)]
    private static extern VisibilityFlags msTransformGetVisibility(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msTransformGetReference(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msTransformGetNumUserProperties(IntPtr self);

    [DllImport(Lib.name)]
    private static extern VariantData msTransformGetUserProperty(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern VariantData msTransformFindUserProperty(IntPtr self, string name);

    [DllImport(Lib.name)]
    private static extern void msTransformSetHostID(IntPtr self, int v);

    [DllImport(Lib.name)]
    private static extern void msTransformSetIndex(IntPtr self, int v);

    [DllImport(Lib.name)]
    private static extern void msTransformSetPath(IntPtr self, string v);

    [DllImport(Lib.name)]
    private static extern void msTransformSetPosition(IntPtr self, Vector3 v);

    [DllImport(Lib.name)]
    private static extern void msTransformSetRotation(IntPtr self, Quaternion v);

    [DllImport(Lib.name)]
    private static extern void msTransformSetScale(IntPtr self, Vector3 v);

    [DllImport(Lib.name)]
    private static extern void msTransformSetVisibility(IntPtr self, VisibilityFlags v);

    [DllImport(Lib.name)]
    private static extern void msTransformSetReference(IntPtr self, string v);

    #endregion

    /// <summary>
    /// Creates a new TransformData and assign the parameter to it
    /// </summary>
    /// <param name="v"></param>
    /// <returns>The newly created TransformData</returns>
    public static explicit operator TransformData(IntPtr v) {
        return new TransformData { self = v };
    }

    /// <summary>
    /// Creates a new CameraData and assign the parameter to it
    /// </summary>
    /// <param name="v"></param>
    /// <returns>The newly created CameraData</returns>
    public static explicit operator CameraData(TransformData v) {
        return new CameraData { self = v.self };
    }

    /// <summary>
    /// Creates a new LightData and assign the parameter to it
    /// </summary>
    /// <param name="v"></param>
    /// <returns>The newly created LightData</returns>
    public static explicit operator LightData(TransformData v) {
        return new LightData { self = v.self };
    }

    /// <summary>
    /// Creates a new MeshData and assign the parameter to it
    /// </summary>
    /// <param name="v"></param>
    /// <returns>The newly created MeshData</returns>
    public static explicit operator MeshData(TransformData v) {
        return new MeshData { self = v.self };
    }

    /// <summary>
    /// Creates a new PointsData and assign the parameter to it
    /// </summary>
    /// <param name="v"></param>
    /// <returns>The newly created PointsData</returns>
    public static explicit operator PointsData(TransformData v) {
        return new PointsData { self = v.self };
    }

    /// <summary>
    /// Creates a new CurvesData and assign the parameter to it
    /// </summary>
    /// <param name="v"></param>
    /// <returns>The newly created CurvesData</returns>
    public static explicit operator CurvesData(TransformData v) {
        return new CurvesData { self = v.self };
    }

    internal static TransformData Create() {
        return msTransformCreate();
    }

    internal TransformDataFlags dataFlags {
        get { return msTransformGetDataFlags(self); }
    }

    internal EntityType entityType {
        get { return msTransformGetType(self); }
    }

    internal int hostID {
        get { return msTransformGetHostID(self); }
        set { msTransformSetHostID(self, value); }
    }

    internal int index {
        get { return msTransformGetIndex(self); }
        set { msTransformSetIndex(self, value); }
    }

    internal string path {
        get { return Misc.S(msTransformGetPath(self)); }
        set { msTransformSetPath(self, value); }
    }

    internal Vector3 position {
        get { return msTransformGetPosition(self); }
        set { msTransformSetPosition(self, value); }
    }

    internal Quaternion rotation {
        get { return msTransformGetRotation(self); }
        set { msTransformSetRotation(self, value); }
    }

    internal Vector3 scale {
        get { return msTransformGetScale(self); }
        set { msTransformSetScale(self, value); }
    }

    internal VisibilityFlags visibility {
        get { return msTransformGetVisibility(self); }
        set { msTransformSetVisibility(self, value); }
    }

    internal string reference {
        get { return Misc.S(msTransformGetReference(self)); }
        set { msTransformSetReference(self, value); }
    }

    internal int numUserData {
        get { return msTransformGetNumUserProperties(self); }
    }

    private VariantData GetUserProperty(int i) {
        return msTransformGetUserProperty(self, i);
    }

    internal VariantData FindUserProperty(string name) {
        return msTransformFindUserProperty(self, name);
    }
}

internal struct CameraDataFlags {
    public BitFlags flags;

    public bool unchanged {
        get { return flags[0]; }
    }

    public bool hasFov {
        get { return flags[2]; }
    }

    public bool hasNearPlane {
        get { return flags[3]; }
    }

    public bool hasFarPlane {
        get { return flags[4]; }
    }

    public bool hasFocalLength {
        get { return flags[5]; }
    }

    public bool hasSensorSize {
        get { return flags[6]; }
    }

    public bool hasLensShift {
        get { return flags[7]; }
    }

    public bool hasViewMatrix {
        get { return flags[8]; }
    }

    public bool hasProjMatrix {
        get { return flags[9]; }
    }
}

/// <summary>
/// CameraData
/// </summary>
[StructLayout(LayoutKind.Explicit)]
internal struct CameraData {
    #region internal

    [FieldOffset(0)] public IntPtr        self;
    [FieldOffset(0)] public TransformData transform;

    [DllImport(Lib.name)]
    private static extern CameraData msCameraCreate();

    [DllImport(Lib.name)]
    private static extern CameraDataFlags msCameraGetDataFlags(IntPtr self);

    [DllImport(Lib.name)]
    private static extern byte msCameraIsOrtho(IntPtr self);

    [DllImport(Lib.name)]
    private static extern float msCameraGetFov(IntPtr self);

    [DllImport(Lib.name)]
    private static extern float msCameraGetNearPlane(IntPtr self);

    [DllImport(Lib.name)]
    private static extern float msCameraGetFarPlane(IntPtr self);

    [DllImport(Lib.name)]
    private static extern float msCameraGetFocalLength(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Vector2 msCameraGetSensorSize(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Vector2 msCameraGetLensShift(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Matrix4x4 msCameraGetViewMatrix(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Matrix4x4 msCameraGetProjMatrix(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msCameraSetOrtho(IntPtr self, byte v);

    [DllImport(Lib.name)]
    private static extern void msCameraSetFov(IntPtr self, float v);

    [DllImport(Lib.name)]
    private static extern void msCameraSetNearPlane(IntPtr self, float v);

    [DllImport(Lib.name)]
    private static extern void msCameraSetFarPlane(IntPtr self, float v);

    [DllImport(Lib.name)]
    private static extern void msCameraSetFocalLength(IntPtr self, float v);

    [DllImport(Lib.name)]
    private static extern void msCameraSetSensorSize(IntPtr self, Vector2 v);

    [DllImport(Lib.name)]
    private static extern void msCameraSetLensShift(IntPtr self, Vector2 v);

    [DllImport(Lib.name)]
    private static extern void msCameraSetViewMatrix(IntPtr self, Matrix4x4 v);

    [DllImport(Lib.name)]
    private static extern void msCameraSetProjMatrix(IntPtr self, Matrix4x4 v);

    #endregion


    /// <summary>
    /// Creates a new CameraData
    /// </summary>
    /// <returns>The newly created CameraData</returns>
    public static CameraData Create() {
        return msCameraCreate();
    }

    internal CameraDataFlags dataFlags {
        get { return msCameraGetDataFlags(self); }
    }

    internal bool orthographic {
        get { return msCameraIsOrtho(self) != 0; }
        set { msCameraSetOrtho(self, (byte)(value ? 1 : 0)); }
    }

    internal float orthographic_size {
        get { return msCameraGetFov(self); }
    }

    internal float fov {
        get { return msCameraGetFov(self); }
        set { msCameraSetFov(self, value); }
    }

    internal float nearPlane {
        get { return msCameraGetNearPlane(self); }
        set { msCameraSetNearPlane(self, value); }
    }

    internal float farPlane {
        get { return msCameraGetFarPlane(self); }
        set { msCameraSetFarPlane(self, value); }
    }

    internal float focalLength {
        get { return msCameraGetFocalLength(self); }
        set { msCameraSetFocalLength(self, value); }
    }

    internal Vector2 sensorSize {
        get { return msCameraGetSensorSize(self); }
        set { msCameraSetSensorSize(self, value); }
    }

    internal Vector2 lensShift {
        get { return msCameraGetLensShift(self); }
        set { msCameraSetLensShift(self, value); }
    }

    internal Matrix4x4 viewMatrix {
        get { return msCameraGetViewMatrix(self); }
        set { msCameraSetViewMatrix(self, value); }
    }

    internal Matrix4x4 projMatrix {
        get { return msCameraGetProjMatrix(self); }
        set { msCameraSetProjMatrix(self, value); }
    }
}

internal struct LightDataFlags {
    public BitFlags flags;

    public bool unchanged {
        get { return flags[0]; }
    }

    public bool hasShadowType {
        get { return flags[2]; }
    }

    public bool hasColor {
        get { return flags[3]; }
    }

    public bool hasIntensity {
        get { return flags[4]; }
    }

    public bool hasRange {
        get { return flags[5]; }
    }

    public bool hasSpotAngle {
        get { return flags[6]; }
    }

    public bool hasLayerMask {
        get { return flags[7]; }
    }
}

/// <summary>
/// LightData
/// </summary>
[StructLayout(LayoutKind.Explicit)]
internal struct LightData {
    #region internal

    [FieldOffset(0)] public IntPtr        self;
    [FieldOffset(0)] public TransformData transform;

    [DllImport(Lib.name)]
    private static extern LightData msLightCreate();

    [DllImport(Lib.name)]
    private static extern LightDataFlags msLightGetDataFlags(IntPtr self);

    [DllImport(Lib.name)]
    private static extern LightType msLightGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msLightSetType(IntPtr self, LightType v);

    [DllImport(Lib.name)]
    private static extern LightShadows msLightGetShadowType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msLightSetShadowType(IntPtr self, LightShadows v);

    [DllImport(Lib.name)]
    private static extern Color msLightGetColor(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msLightSetColor(IntPtr self, Color v);

    [DllImport(Lib.name)]
    private static extern float msLightGetIntensity(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msLightSetIntensity(IntPtr self, float v);

    [DllImport(Lib.name)]
    private static extern float msLightGetRange(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msLightSetRange(IntPtr self, float v);

    [DllImport(Lib.name)]
    private static extern float msLightGetSpotAngle(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msLightSetSpotAngle(IntPtr self, float v);

    #endregion

    /// <summary>
    /// Creates a new LightData
    /// </summary>
    /// <returns>The newly created LightData</returns>
    public static LightData Create() {
        return msLightCreate();
    }

    internal LightDataFlags dataFlags {
        get { return msLightGetDataFlags(self); }
    }

    internal LightType lightType {
        get { return msLightGetType(self); }
        set { msLightSetType(self, value); }
    }

    internal LightShadows shadowType {
        get { return msLightGetShadowType(self); }
        set { msLightSetShadowType(self, value); }
    }

    internal Color color {
        get { return msLightGetColor(self); }
        set { msLightSetColor(self, value); }
    }

    internal float intensity {
        get { return msLightGetIntensity(self); }
        set { msLightSetIntensity(self, value); }
    }

    internal float range {
        get { return msLightGetRange(self); }
        set { msLightSetRange(self, value); }
    }

    internal float spotAngle {
        get { return msLightGetSpotAngle(self); }
        set { msLightSetSpotAngle(self, value); }
    }
}

#region Mesh

internal struct SubmeshData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern int msSubmeshGetNumIndices(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msSubmeshReadIndices(IntPtr self, IntPtr mesh, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern int msSubmeshGetMaterialID(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Topology msSubmeshGetTopology(IntPtr self);

    #endregion

    public enum Topology {
        Points,
        Lines,
        Triangles,
        Quads
    };


    public int numIndices {
        get { return msSubmeshGetNumIndices(self); }
    }

    public Topology topology {
        get { return msSubmeshGetTopology(self); }
    }

    public int materialID {
        get { return msSubmeshGetMaterialID(self); }
    }

    internal void ReadIndices(MeshData mesh, PinnedList<int> dst) {
        msSubmeshReadIndices(self, mesh.self, dst);
    }
}

internal struct BlendShapeData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern IntPtr msBlendShapeGetName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msBlendShapeSetName(IntPtr self, string v);

    [DllImport(Lib.name)]
    private static extern float msBlendShapeGetWeight(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msBlendShapeSetWeight(IntPtr self, float v);

    [DllImport(Lib.name)]
    private static extern int msBlendShapeGetNumFrames(IntPtr self);

    [DllImport(Lib.name)]
    private static extern float msBlendShapeGetFrameWeight(IntPtr self, int f);

    [DllImport(Lib.name)]
    private static extern void msBlendShapeReadPoints(IntPtr self, int f, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msBlendShapeReadNormals(IntPtr self, int f, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msBlendShapeReadTangents(IntPtr self, int f, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msBlendShapeAddFrame(IntPtr self, float weight, int num, Vector3[] v, Vector3[] n, Vector3[] t);

    #endregion

    public string name {
        get { return Misc.S(msBlendShapeGetName(self)); }
        set { msBlendShapeSetName(self, value); }
    }

    public float weight {
        get { return msBlendShapeGetWeight(self); }
        set { msBlendShapeSetWeight(self, value); }
    }

    public float numFrames {
        get { return msBlendShapeGetNumFrames(self); }
    }

    public float GetWeight(int f) {
        return msBlendShapeGetFrameWeight(self, f);
    }

    internal void ReadPoints(int f, PinnedList<Vector3> dst) {
        msBlendShapeReadPoints(self, f, dst);
    }

    internal void ReadNormals(int f, PinnedList<Vector3> dst) {
        msBlendShapeReadNormals(self, f, dst);
    }

    internal void ReadTangents(int f, PinnedList<Vector3> dst) {
        msBlendShapeReadTangents(self, f, dst);
    }

    public void AddFrame(float w, Vector3[] v, Vector3[] n, Vector3[] t) {
        msBlendShapeAddFrame(self, w, v.Length, v, n, t);
    }
}

internal struct MeshDataFlags {
    public BitFlags flags;

    public bool unchanged {
        get { return flags[0]; }
    }

    public bool topologyUnchanged {
        get { return flags[1]; }
    }

    public bool hasIndices {
        get { return flags[3]; }
    }

    public bool hasPoints {
        get { return flags[5]; }
    }

    public bool hasNormals {
        get { return flags[6]; }
    }

    public bool hasTangents {
        get { return flags[7]; }
    }

    public bool hasColors {
        get { return flags[10]; }
    }

    public bool hasVelocities {
        get { return flags[11]; }
    }

    public bool hasRootBone {
        get { return flags[14]; }
    }

    public bool hasBones {
        get { return flags[15]; }
    }

    public bool hasBlendshapes {
        get { return flags[16]; }
    }

    public bool hasBlendshapeWeights {
        get { return flags[17]; }
    }

    public bool hasBounds {
        get { return flags[19]; }
    }

    private const int UV_START_BIT_POS = 24;

    public bool HasUV(int index) {
        return flags[UV_START_BIT_POS + index];
    }
};

/// <summary>
/// MeshData
/// </summary>
[StructLayout(LayoutKind.Explicit)]
internal struct MeshData {
    #region internal

    [FieldOffset(0)] public IntPtr        self;
    [FieldOffset(0)] public TransformData transform;

    [DllImport(Lib.name)]
    private static extern MeshData msMeshCreate();

    [DllImport(Lib.name)]
    private static extern MeshDataFlags msMeshGetDataFlags(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msMeshSetFlags(IntPtr self, MeshDataFlags v);

    [DllImport(Lib.name)]
    private static extern int msMeshGetNumPoints(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msMeshGetNumIndices(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msMeshReadPoints(IntPtr self, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msMeshReadNormals(IntPtr self, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msMeshReadTangents(IntPtr self, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msMeshReadUV(IntPtr self, IntPtr dst, int index);

    [DllImport(Lib.name)]
    private static extern void msMeshReadColors(IntPtr self, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msMeshReadVelocities(IntPtr self, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msMeshReadIndices(IntPtr self, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetPointsPtr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetNormalsPtr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetTangentsPtr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetUV0Ptr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetUV1Ptr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetColorsPtr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetVelocitiesPtr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetIndicesPtr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msMeshWritePoints(IntPtr self, Vector3[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteNormals(IntPtr self, Vector3[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteTangents(IntPtr self, Vector4[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteUV(IntPtr self, int index, Vector2[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteColors(IntPtr self, Color[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteVelocities(IntPtr self, Vector3[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteIndices(IntPtr self, int[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteSubmeshTriangles(IntPtr self, int[] v, int size, int materialID);

    [DllImport(Lib.name)]
    private static extern int msMeshGetNumSubmeshes(IntPtr self);

    [DllImport(Lib.name)]
    private static extern SubmeshData msMeshGetSubmesh(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern Bounds msMeshGetBounds(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msMeshReadBoneWeights4(IntPtr self, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteBoneWeights4(IntPtr self, BoneWeight[] weights, int size);
#if UNITY_2019_1_OR_NEWER
    [DllImport(Lib.name)]
    private static extern void msMeshReadBoneCounts(IntPtr self, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern void msMeshReadBoneWeightsV(IntPtr self, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetBoneCountsPtr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetBoneWeightsVPtr(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteBoneWeightsV(IntPtr self, IntPtr counts, int numCounts, IntPtr weights,
        int numWeights);
#endif
    [DllImport(Lib.name)]
    private static extern int msMeshGetNumBones(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msMeshGetNumBoneWeights(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetRootBonePath(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msMeshSetRootBonePath(IntPtr self, string v);

    [DllImport(Lib.name)]
    private static extern IntPtr msMeshGetBonePath(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern void msMeshSetBonePath(IntPtr self, string v, int i);

    [DllImport(Lib.name)]
    private static extern void msMeshReadBindPoses(IntPtr self, Matrix4x4[] v);

    [DllImport(Lib.name)]
    private static extern void msMeshWriteBindPoses(IntPtr self, Matrix4x4[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msMeshSetLocal2World(IntPtr self, ref Matrix4x4 v);

    [DllImport(Lib.name)]
    private static extern void msMeshSetWorld2Local(IntPtr self, ref Matrix4x4 v);


    [DllImport(Lib.name)]
    private static extern int msMeshGetNumBlendShapes(IntPtr self);

    [DllImport(Lib.name)]
    private static extern BlendShapeData msMeshGetBlendShapeData(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern BlendShapeData msMeshAddBlendShape(IntPtr self, string name);

    #endregion

    /// <summary>
    /// Create MeshData
    /// </summary>
    /// <returns>The newly created MeshData </returns>
    public static MeshData Create() {
        return msMeshCreate();
    }

    internal MeshDataFlags dataFlags {
        get { return msMeshGetDataFlags(self); }
        set { msMeshSetFlags(self, value); }
    }

    internal int numPoints {
        get { return msMeshGetNumPoints(self); }
    }

    internal int numIndices {
        get { return msMeshGetNumIndices(self); }
    }

    internal Bounds bounds {
        get { return msMeshGetBounds(self); }
    }

    internal void ReadPoints(PinnedList<Vector3> dst) {
        msMeshReadPoints(self, dst);
    }

    internal void ReadNormals(PinnedList<Vector3> dst) {
        msMeshReadNormals(self, dst);
    }

    internal void ReadTangents(PinnedList<Vector4> dst) {
        msMeshReadTangents(self, dst);
    }

    internal void ReadUV(PinnedList<Vector2> dst, int index) {
        msMeshReadUV(self, dst, index);
    }

    internal void ReadColors(PinnedList<Color> dst) {
        msMeshReadColors(self, dst);
    }

    internal void ReadVelocities(PinnedList<Vector3> dst) {
        msMeshReadVelocities(self, dst);
    }

    internal void ReadBoneWeights4(IntPtr dst) {
        msMeshReadBoneWeights4(self, dst);
    }
#if UNITY_2019_1_OR_NEWER
    internal void ReadBoneCounts(IntPtr dst) {
        msMeshReadBoneCounts(self, dst);
    }

    internal void ReadBoneWeightsV(IntPtr dst) {
        msMeshReadBoneWeightsV(self, dst);
    }
#endif
    internal void ReadIndices(IntPtr dst) {
        msMeshReadIndices(self, dst);
    }

    internal void WritePoints(Vector3[] v) {
        msMeshWritePoints(self, v, v.Length);
    }

    internal void WriteNormals(Vector3[] v) {
        msMeshWriteNormals(self, v, v.Length);
    }

    internal void WriteTangents(Vector4[] v) {
        msMeshWriteTangents(self, v, v.Length);
    }

    internal void WriteUV(int index, Vector2[] v) {
        msMeshWriteUV(self, index, v, v.Length);
    }

    internal void WriteColors(Color[] v) {
        msMeshWriteColors(self, v, v.Length);
    }

    internal void WriteVelocities(Vector3[] v) {
        msMeshWriteVelocities(self, v, v.Length);
    }

    internal void WriteBoneWeights4(BoneWeight[] v) {
        msMeshWriteBoneWeights4(self, v, v.Length);
    }
#if UNITY_2019_1_OR_NEWER
    internal void WriteBoneWeightsV(ref NativeArray<byte> counts, ref NativeArray<BoneWeight1> weights) {
        msMeshWriteBoneWeightsV(self, Misc.ForceGetPointer(ref counts), counts.Length,
            Misc.ForceGetPointer(ref weights), weights.Length);
    }
#endif
    internal void WriteIndices(int[] v) {
        msMeshWriteIndices(self, v, v.Length);
    }

    internal Matrix4x4 local2world {
        set { msMeshSetLocal2World(self, ref value); }
    }

    internal Matrix4x4 world2local {
        set { msMeshSetWorld2Local(self, ref value); }
    }

    internal void WriteSubmeshTriangles(int[] indices, int materialID) {
        msMeshWriteSubmeshTriangles(self, indices, indices.Length, materialID);
    }

    internal int numBones {
        get { return msMeshGetNumBones(self); }
    }

    internal string[] bonePaths {
        get {
            int      n   = numBones;
            string[] ret = new string[n];
            for (int i = 0; i < n; ++i)
                ret[i] = Misc.S(msMeshGetBonePath(self, i));
            return ret;
        }
    }

    internal string rootBonePath {
        get { return Misc.S(msMeshGetRootBonePath(self)); }
        set { msMeshSetRootBonePath(self, value); }
    }

    internal int numBoneWeights {
        get { return msMeshGetNumBoneWeights(self); }
    }

    internal int numSubmeshes {
        get { return msMeshGetNumSubmeshes(self); }
    }

    internal int numBlendShapes {
        get { return msMeshGetNumBlendShapes(self); }
    }

    internal Matrix4x4[] bindposes {
        get {
            Matrix4x4[] ret = new Matrix4x4[numBones];
            msMeshReadBindPoses(self, ret);
            return ret;
        }
        set { msMeshWriteBindPoses(self, value, value.Length); }
    }

    internal void SetBonePaths(BaseMeshSync mss, Transform[] bones) {
        int n = bones.Length;
        for (int i = 0; i < n; ++i) {
            string path = mss.BuildPath(bones[i]);
            msMeshSetBonePath(self, path, i);
        }
    }

    internal SubmeshData GetSubmesh(int i) {
        return msMeshGetSubmesh(self, i);
    }

    internal BlendShapeData GetBlendShapeData(int i) {
        return msMeshGetBlendShapeData(self, i);
    }

    internal BlendShapeData AddBlendShape(string name) {
        return msMeshAddBlendShape(self, name);
    }
};

#endregion

#region Point

internal struct PointsDataFlags {
    public BitFlags flags;

    public bool unchanged {
        get { return flags[0]; }
    }

    public bool topologyUnchanged {
        get { return flags[1]; }
    }

    public bool hasPoints {
        get { return flags[2]; }
    }

    public bool hasRotations {
        get { return flags[3]; }
    }

    public bool hasScales {
        get { return flags[4]; }
    }

    public bool hasColors {
        get { return flags[5]; }
    }

    public bool hasVelocities {
        get { return flags[6]; }
    }

    public bool hasIDs {
        get { return flags[7]; }
    }
};

/// <summary>
/// PointsData
/// </summary>
[StructLayout(LayoutKind.Explicit)]
internal struct PointsData {
    #region internal

    [FieldOffset(0)] public IntPtr        self;
    [FieldOffset(0)] public TransformData transform;

    [DllImport(Lib.name)]
    private static extern PointsData msPointsCreate();

    [DllImport(Lib.name)]
    private static extern PointsDataFlags msPointsGetFlags(IntPtr self);

    [DllImport(Lib.name)]
    private static extern Bounds msPointsGetBounds(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msPointsGetNumPoints(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msPointsReadPoints(IntPtr self, Vector3[] dst);

    [DllImport(Lib.name)]
    private static extern void msPointsWritePoints(IntPtr self, Vector3[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msPointsReadRotations(IntPtr self, Quaternion[] dst);

    [DllImport(Lib.name)]
    private static extern void msPointsWriteRotations(IntPtr self, Quaternion[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msPointsReadScales(IntPtr self, Vector3[] dst);

    [DllImport(Lib.name)]
    private static extern void msPointsWriteScales(IntPtr self, Vector3[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msPointsReadVelocities(IntPtr self, Vector3[] dst);

    [DllImport(Lib.name)]
    private static extern void msPointsWriteVelocities(IntPtr self, Vector3[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msPointsReadColors(IntPtr self, Color[] dst);

    [DllImport(Lib.name)]
    private static extern void msPointsWriteColors(IntPtr self, Color[] v, int size);

    [DllImport(Lib.name)]
    private static extern void msPointsReadIDs(IntPtr self, int[] dst);

    [DllImport(Lib.name)]
    private static extern void msPointsWriteIDs(IntPtr self, int[] v, int size);

    #endregion

    /// <summary>
    /// Creates a new PointsData
    /// </summary>
    /// <returns>The newly created PointsData </returns>
    public static PointsData Create() {
        return msPointsCreate();
    }

    internal PointsDataFlags dataFlags {
        get { return msPointsGetFlags(self); }
    }

    internal Bounds bounds {
        get { return msPointsGetBounds(self); }
    }

    internal int numPoints {
        get { return msPointsGetNumPoints(self); }
    }

    internal void ReadPoints(Vector3[] dst) {
        msPointsReadPoints(self, dst);
    }

    internal void ReadRotations(Quaternion[] dst) {
        msPointsReadRotations(self, dst);
    }

    internal void ReadScales(Vector3[] dst) {
        msPointsReadScales(self, dst);
    }

    internal void ReadVelocities(Vector3[] dst) {
        msPointsReadVelocities(self, dst);
    }

    internal void ReadColors(Color[] dst) {
        msPointsReadColors(self, dst);
    }

    internal void ReadIDs(int[] dst) {
        msPointsReadIDs(self, dst);
    }

    internal void WritePoints(Vector3[] v) {
        msPointsWritePoints(self, v, v.Length);
    }

    internal void WriteRotations(Quaternion[] v) {
        msPointsWriteRotations(self, v, v.Length);
    }

    internal void WriteScales(Vector3[] v) {
        msPointsWriteScales(self, v, v.Length);
    }

    internal void WriteVelocities(Vector3[] v) {
        msPointsWriteVelocities(self, v, v.Length);
    }

    internal void WriteColors(Color[] v) {
        msPointsWriteColors(self, v, v.Length);
    }

    internal void WriteIDs(int[] v) {
        msPointsWriteIDs(self, v, v.Length);
    }
}

#endregion

#region Curve

/// <summary>
/// CurvesData
/// </summary>
[StructLayout(LayoutKind.Explicit)]
internal struct CurvesData {
    #region internal

    [FieldOffset(0)] public IntPtr        self;
    [FieldOffset(0)] public TransformData transform;


    [DllImport(Lib.name)]
    private static extern int msCurveGetNumSplines(IntPtr self);

    [DllImport(Lib.name)]
    private static extern bool msCurveReadSplineClosed(IntPtr self, int index);

    [DllImport(Lib.name)]
    private static extern int msCurveGetNumSplinePoints(IntPtr self, int index);

    [DllImport(Lib.name)]
    private static extern int msCurveReadSplineCos(IntPtr self, int index, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern int msCurveReadSplineHandlesLeft(IntPtr self, int index, IntPtr dst);

    [DllImport(Lib.name)]
    private static extern int msCurveReadSplineHandlesRight(IntPtr self, int index, IntPtr dst);

    #endregion

#if AT_USE_SPLINES
        public int numSplines
        {
            get { return msCurveGetNumSplines(self); }
        }

        public bool IsSplineClosed(int index)
        {
            return msCurveReadSplineClosed(self, index);
        }        

        public int GetNumSplinePoints(int index)
        {
            return msCurveGetNumSplinePoints(self, index);
        }

        public void ReadSplineCos(int index, PinnedList<float3> dst)
        {
            msCurveReadSplineCos(self, index, dst);
        }

        public void ReadSplineHandlesLeft(int index, PinnedList<float3> dst)
        {
            msCurveReadSplineHandlesLeft(self, index, dst);
        }

        public void ReadSplineHandlesRight(int index, PinnedList<float3> dst)
        {
            msCurveReadSplineHandlesRight(self, index, dst);
        }
#endif
}

#endregion Curve

#endregion


#region Constraints

internal struct ConstraintData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern ConstraintType msConstraintGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msConstraintGetPath(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msConstraintGetNumSources(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msConstraintGetSource(IntPtr self, int i);

    #endregion

    internal enum ConstraintType {
        Unknown,
        Aim,
        Parent,
        Position,
        Rotation,
        Scale
    }

    /// <summary>
    /// Assigns a new ConstraintData with the parameter
    /// </summary>
    /// <param name="v"></param>
    /// <returns>The new ConstraintData</returns>
    public static explicit operator ConstraintData(IntPtr v) {
        ConstraintData ret;
        ret.self = v;
        return ret;
    }

    public ConstraintType type {
        get { return msConstraintGetType(self); }
    }

    public string path {
        get { return Misc.S(msConstraintGetPath(self)); }
    }

    public int numSources {
        get { return msConstraintGetNumSources(self); }
    }

    public string GetSourcePath(int i) {
        return Misc.S(msConstraintGetSource(self, i));
    }
}

internal struct AimConstraintData {
    #region internal

    public IntPtr self;

    #endregion


    public static explicit operator AimConstraintData(ConstraintData v) {
        AimConstraintData ret;
        ret.self = v.self;
        return ret;
    }
}

internal struct ParentConstraintData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern Vector3 msParentConstraintGetPositionOffset(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern Quaternion msParentConstraintGetRotationOffset(IntPtr self, int i);

    #endregion


    public static explicit operator ParentConstraintData(ConstraintData v) {
        ParentConstraintData ret;
        ret.self = v.self;
        return ret;
    }

    public Vector3 GetPositionOffset(int i) {
        return msParentConstraintGetPositionOffset(self, i);
    }

    public Quaternion GetRotationOffset(int i) {
        return msParentConstraintGetRotationOffset(self, i);
    }
}

internal struct PositionConstraintData {
    #region internal

    public IntPtr self;

    #endregion

    public static explicit operator PositionConstraintData(ConstraintData v) {
        PositionConstraintData ret;
        ret.self = v.self;
        return ret;
    }
}

internal struct RotationConstraintData {
    #region internal

    public IntPtr self;

    #endregion

    public static explicit operator RotationConstraintData(ConstraintData v) {
        RotationConstraintData ret;
        ret.self = v.self;
        return ret;
    }
}

internal struct ScaleConstrainData {
    #region internal

    public IntPtr self;

    #endregion

    public static explicit operator ScaleConstrainData(ConstraintData v) {
        ScaleConstrainData ret;
        ret.self = v.self;
        return ret;
    }
}

#endregion


#region Scene

internal enum ZUpCorrectionMode {
    FlipYZ,
    RotateX
}

internal struct SceneProfileData {
    public ulong sizeEncoded;
    public ulong sizeDecoded;
    public ulong vertexCount;
    public float loadTime;   // in ms
    public float readTime;   // in ms
    public float decodeTime; // in ms
    public float setupTime;  // in ms
    public float lerpTime;   // in ms
};

#region InstanceInfo

internal struct InstanceInfoData {
    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern IntPtr msInstanceInfoGetPath(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msInstanceInfoGetParentPath(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msInstanceInfoPropGetArrayLength(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msInstanceInfoCopyTransforms(IntPtr self, IntPtr matrices);

    public string path {
        get { return Misc.S(msInstanceInfoGetPath(self)); }
    }

    public string parentPath {
        get { return Misc.S(msInstanceInfoGetParentPath(self)); }
    }

    public int arrayLength {
        get { return msInstanceInfoPropGetArrayLength(self); }
    }

    public Matrix4x4[] transforms {
        get {
            using (PinnedList<Matrix4x4> pinnedData = new PinnedList<Matrix4x4>(arrayLength)) {
                msInstanceInfoCopyTransforms(self, pinnedData.Pointer);
                return pinnedData.Array;
            }
        }
    }
}

#endregion


#region PropertyInfo

internal enum PropertyInfoDataType {
    Int,
    Float,
    IntArray,
    FloatArray,
    String
}

internal enum PropertyInfoDataSourceType {
    GEO_NODES,
    CUSTOM_PROPERTY
};

internal struct PropertyInfoData {
    public IntPtr self;

    #region DLL Imports

    [DllImport(Lib.name)]
    private static extern IntPtr msPropertyInfoGetPath(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msPropertyInfoGetName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msPropertyInfoGetModifierName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern IntPtr msPropertyInfoGetPropertyName(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msPropertyInfoGetType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msPropertyInfoGetSourceType(IntPtr self);

    [DllImport(Lib.name)]
    private static extern float msPropertyInfoGetMin(IntPtr self);

    [DllImport(Lib.name)]
    private static extern float msPropertyInfoGetMax(IntPtr self);

    #endregion DLL Imports

    public float min {
        get { return msPropertyInfoGetMin(self); }
    }

    public float max {
        get { return msPropertyInfoGetMax(self); }
    }

    public string path {
        get { return Misc.S(msPropertyInfoGetPath(self)); }
    }

    public string modifierName {
        get { return Misc.S(msPropertyInfoGetModifierName(self)); }
    }

    public string propertyName {
        get { return Misc.S(msPropertyInfoGetPropertyName(self)); }
    }

    public string name {
        get { return Misc.S(msPropertyInfoGetName(self)); }
    }

    public PropertyInfoDataType type {
        get { return (PropertyInfoDataType)msPropertyInfoGetType(self); }
    }

    public PropertyInfoDataSourceType sourceType {
        get { return (PropertyInfoDataSourceType)msPropertyInfoGetSourceType(self); }
    }
}

#endregion PropertyInfo


internal struct SceneData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern int msSceneGetNumAssets(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AssetData msSceneGetAsset(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern int msSceneGetNumEntities(IntPtr self);

    [DllImport(Lib.name)]
    private static extern TransformData msSceneGetEntity(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern int msSceneGetNumConstraints(IntPtr self);

    [DllImport(Lib.name)]
    private static extern ConstraintData msSceneGetConstraint(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern byte msSceneSubmeshesHaveUniqueMaterial(IntPtr self);

    [DllImport(Lib.name)]
    private static extern SceneProfileData msSceneGetProfileData(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msSceneGetNumInstanceInfos(IntPtr self);

    [DllImport(Lib.name)]
    private static extern InstanceInfoData msSceneGetInstanceInfo(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern int msSceneGetNumInstanceMeshes(IntPtr self);

    [DllImport(Lib.name)]
    private static extern TransformData msSceneGetInstanceMesh(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern int msSceneGetNumPropertyInfos(IntPtr self);

    [DllImport(Lib.name)]
    private static extern PropertyInfoData msSceneGetPropertyInfo(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern int msSceneGetMaterialSyncMode(IntPtr self);

    #endregion

    public static implicit operator bool(SceneData v) {
        return v.self != IntPtr.Zero;
    }

    public int numAssets {
        get { return msSceneGetNumAssets(self); }
    }

    public int numEntities {
        get { return msSceneGetNumEntities(self); }
    }

    public int numConstraints {
        get { return msSceneGetNumConstraints(self); }
    }

    public int numInstanceInfos {
        get { return msSceneGetNumInstanceInfos(self); }
    }

    public int numInstancedEntities {
        get { return msSceneGetNumInstanceMeshes(self); }
    }

    public int numPropertyInfos {
        get { return msSceneGetNumPropertyInfos(self); }
    }


    public bool submeshesHaveUniqueMaterial {
        get { return msSceneSubmeshesHaveUniqueMaterial(self) != 0; }
    }

    public SceneProfileData profileData {
        get { return msSceneGetProfileData(self); }
    }

    public AssetData GetAsset(int i) {
        return msSceneGetAsset(self, i);
    }

    public TransformData GetEntity(int i) {
        return msSceneGetEntity(self, i);
    }

    public ConstraintData GetConstraint(int i) {
        return msSceneGetConstraint(self, i);
    }

    public InstanceInfoData GetInstanceInfo(int i) {
        return msSceneGetInstanceInfo(self, i);
    }

    public TransformData GetInstancedEntity(int i) {
        return msSceneGetInstanceMesh(self, i);
    }

    public PropertyInfoData GetPropertyInfo(int i) {
        return msSceneGetPropertyInfo(self, i);
    }

    public string GetMaterialSyncMode() {
        int mode = 0;
        // The method might not exist, we can't bump the protocol version yet:
        try {
            mode = msSceneGetMaterialSyncMode(self);
        }
        catch {
        }

        switch (mode) {
            case 1:
                return "Basic";
            default:
                return "None";
        }
    }
}

#endregion Scene

#region SceneCache

internal struct SceneCacheData {
    #region internal

    public IntPtr self;

    [DllImport(Lib.name)]
    private static extern SceneCacheData msSceneCacheOpen(string path);

    [DllImport(Lib.name)]
    private static extern void msSceneCacheClose(IntPtr self);

    [DllImport(Lib.name)]
    private static extern int msSceneCacheGetPreloadLength(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msSceneCacheSetPreloadLength(IntPtr self, int v);

    [DllImport(Lib.name)]
    private static extern float msSceneCacheGetSampleRate(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msSceneCacheGetTimeRange(IntPtr self, ref float start, ref float end);

    [DllImport(Lib.name)]
    private static extern int msSceneCacheGetNumScenes(IntPtr self);

    [DllImport(Lib.name)]
    private static extern float msSceneCacheGetTime(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern int msSceneCacheGetFrameByTime(IntPtr self, float time);

    [DllImport(Lib.name)]
    private static extern SceneData msSceneCacheLoadByFrame(IntPtr self, int i);

    [DllImport(Lib.name)]
    private static extern SceneData msSceneCacheLoadByTime(IntPtr self, float time, bool lerp);

    [DllImport(Lib.name)]
    private static extern void msSceneCacheRefresh(IntPtr self);

    [DllImport(Lib.name)]
    private static extern void msSceneCachePreload(IntPtr self, int frame);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msSceneCacheGetTimeCurve(IntPtr self);

    [DllImport(Lib.name)]
    private static extern AnimationCurveData msSceneCacheGetFrameCurve(IntPtr self);

    #endregion

    public static implicit operator bool(SceneCacheData v) {
        return v.self != IntPtr.Zero;
    }

    public static SceneCacheData Open(string path) {
        return msSceneCacheOpen(path);
    }

    public void Close() {
        msSceneCacheClose(self);
        self = IntPtr.Zero;
    }

    public int GetPreloadLength() {
        return msSceneCacheGetPreloadLength(self);
    }

    public void SetPreloadLength(int preload) {
        msSceneCacheSetPreloadLength(self, preload);
    }

    public float GetSampleRate() {
        return msSceneCacheGetSampleRate(self);
    }

    public int GetNumScenes() {
        return msSceneCacheGetNumScenes(self);
    }

    internal TimeRange GetTimeRange() {
        TimeRange ret = default;
        msSceneCacheGetTimeRange(self, ref ret.start, ref ret.end);
        return ret;
    }

    public float GetTime(int i) {
        return msSceneCacheGetTime(self, i);
    }

    public int GetFrame(float time) {
        return msSceneCacheGetFrameByTime(self, time);
    }

    internal SceneData LoadByFrame(int i) {
        return msSceneCacheLoadByFrame(self, i);
    }

    internal SceneData LoadByTime(float t, bool lerp) {
        return msSceneCacheLoadByTime(self, t, lerp);
    }

    public void Refresh() {
        msSceneCacheRefresh(self);
    }

    public void Preload(int frame) {
        msSceneCachePreload(self, frame);
    }

//----------------------------------------------------------------------------------------------------------------------        
    internal AnimationCurve GetTimeCurve(InterpolationMode im) {
        AnimationCurveData data = msSceneCacheGetTimeCurve(self);
        return !data ? null : CreateAnimationCurveFromData(data, im);
    }

    public AnimationCurve GetFrameCurve() {
        AnimationCurveData data = msSceneCacheGetFrameCurve(self);
        return !data ? null : CreateAnimationCurveFromData(data, InterpolationMode.Constant);
    }

//----------------------------------------------------------------------------------------------------------------------        
    private static AnimationCurve CreateAnimationCurveFromData(AnimationCurveData curveData,
        InterpolationMode interpolationMode) {
        AnimationClipData.Prepare();
        curveData.Convert(interpolationMode);
        Keyframe[] keys = new Keyframe[curveData.GetKeyCount(0)];
        curveData.Copy(0, keys);
        return new AnimationCurve(keys);
    }
}

#endregion
} //end namespace