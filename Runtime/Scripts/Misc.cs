using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using JetBrains.Annotations;
using UnityEngine;
using Unity.Collections;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync {
internal static class Misc {
    #region internal

    [DllImport(Lib.name)]
    private static extern ulong msGetTime();

    [DllImport(Lib.name)]
    private static extern int msKeyframeReduction(Keyframe[] keys, int keyCount, float threshold, byte eraseFlatCurves);

    #endregion

    public static ulong GetTimeNS() {
        return msGetTime();
    }

    public static float NS2MS(ulong ns) {
        return (float)((double)ns / 1000000.0);
    }

    public static byte ToByte(bool v) {
        return v ? (byte)1 : (byte)0;
    }

    public static string S(IntPtr cstring) {
        return cstring == IntPtr.Zero ? "" : Marshal.PtrToStringAnsi(cstring);
    }

    public static string SanitizeFileName(string name) {
        Regex reg = new Regex("[:<>|\\*\\?]");
        return reg.Replace(name, "_");
    }

    public static void Resize<T>(List<T> list, int n) where T : new() {
        int cur = list.Count;
        if (n < cur) {
            list.RemoveRange(n, cur - n);
        }
        else if (n > cur) {
            if (n > list.Capacity)
                list.Capacity = n;
            int a = n - cur;
            for (int i = 0; i < a; ++i)
                list.Add(new T());
        }
    }

    public static T GetOrAddComponent<T>(GameObject go) where T : Component {
        T ret = go.GetComponent<T>();
        if (ret == null)
            ret = go.AddComponent<T>();
        return ret;
    }

    public static Component GetOrAddComponent(GameObject go, Type type) {
        Component ret = go.GetComponent(type);
        if (ret == null)
            ret = go.AddComponent(type);
        return ret;
    }

#if UNITY_EDITOR

    //[TODO-sin: 2022-3-14] Check if msKeyframeReduction is used
    private static Keyframe[] KeyframeReduction(Keyframe[] keys, float threshold, bool eraseFlatCurves) {
        AnimationClipData.Prepare();
        int        newCount = msKeyframeReduction(keys, keys.Length, threshold, ToByte(eraseFlatCurves));
        Keyframe[] newKeys  = new Keyframe[newCount];
        if (newCount > 0)
            Array.Copy(keys, newKeys, newCount);
        return newKeys;
    }

    public static void KeyframeReduction(AnimationCurve curve, float threshold, bool eraseFlatCurves) {
        if (curve.length <= 2)
            return;
        curve.keys = KeyframeReduction(curve.keys, threshold, eraseFlatCurves);
    }

    public static void SetCurve(AnimationClip clip, string path, Type type, string prop, AnimationCurve curve) {
        //var binding = EditorCurveBinding.FloatCurve(path, type, prop);
        //AnimationUtility.SetEditorCurve(clip, binding, curve);

        // AnimationUtility.SetEditorCurve() is far too slow and impractical (as of Unity 2019.2)
        // we stick to the old way for now.
        clip.SetCurve(path, type, prop, curve);
    }

    public static void SetCurve(AnimationClip clip, EditorCurveBinding binding, AnimationCurve curve) {
        // see above
        //AnimationUtility.SetEditorCurve(clip, binding, curve);
        clip.SetCurve(binding.path, binding.type, binding.propertyName, curve);
    }

//----------------------------------------------------------------------------------------------------------------------

    // copy a file to StreamingAssets.
    // if here is already a file with same name:
    // - use existing one if both are identical (identical = same file size & same mtime)
    // - otherwise copy with rename (e.g. hoge.png -> hoge2.png)
    // return destination path
    public static string CopyFileToStreamingAssets(string srcPath) {
        if (!File.Exists(srcPath))
            return null; // srcPath doesn't exist

        string streamingAssetsPath = Application.streamingAssetsPath;
        if (srcPath.Contains(streamingAssetsPath))
            return srcPath; // srcPath is already in StreamingAssets

        string filename = Path.GetFileNameWithoutExtension(srcPath);
        string ext      = Path.GetExtension(srcPath);
        for (int n = 1;; ++n) {
            string ns      = n == 1 ? "" : n.ToString();
            string dstPath = string.Format("{0}/{1}{2}{3}", streamingAssetsPath, filename, ns, ext);
            if (File.Exists(dstPath)) {
                FileInfo srcInfo = new FileInfo(srcPath);
                FileInfo dstInfo = new FileInfo(dstPath);
                bool identical =
                    srcInfo.Length == dstInfo.Length &&
                    srcInfo.LastWriteTime == dstInfo.LastWriteTime;
                if (!identical)
                    continue;

                // use existing one
                Debug.Log($"CopyFileToStreamingAssets: use existing file {srcPath} -> {dstPath}");
                return dstPath;
            }
            else {
                // do copy
                File.Copy(srcPath, dstPath);
                Debug.Log($"CopyFileToStreamingAssets: copy {srcPath} -> {dstPath}");
                AssetDatabase.Refresh();
                return dstPath;
            }
        }
    }

//----------------------------------------------------------------------------------------------------------------------        

    public static string MoveFileToStreamingAssets(string srcPath) {
        if (!File.Exists(srcPath))
            return null; // srcPath doesn't exist

        string streaminAssetsPath = Application.streamingAssetsPath;
        if (srcPath.Contains(streaminAssetsPath))
            return srcPath; // srcPath is already in StreamingAssets

        string filename = Path.GetFileNameWithoutExtension(srcPath);
        string ext      = Path.GetExtension(srcPath);
        for (int n = 1;; ++n) {
            string ns      = n == 1 ? "" : n.ToString();
            string dstPath = string.Format("{0}/{1}{2}{3}", streaminAssetsPath, filename, ns, ext);
            if (File.Exists(dstPath)) {
                FileInfo srcInfo = new FileInfo(srcPath);
                FileInfo dstInfo = new FileInfo(dstPath);
                bool identical =
                    srcInfo.Length == dstInfo.Length &&
                    srcInfo.LastWriteTime == dstInfo.LastWriteTime;
                if (identical) {
                    // use existing one
                    Debug.Log(
                        string.Format("MoveFileToStreamingAssets: use existing file {0} -> {1}", srcPath, dstPath));
                    return dstPath;
                }
                else {
                    continue;
                }
            }
            else {
                // do move
                File.Move(srcPath, dstPath);
                Debug.Log(string.Format("MoveFileToStreamingAssets: move {0} -> {1}", srcPath, dstPath));
                return dstPath;
            }
        }
    }

    // save asset to path.
    // if there is already a file, overwrite it but keep .meta intact.
    [CanBeNull]
    public static T OverwriteOrCreateAsset<T>(T asset, string path) where T : UnityEngine.Object {
        try {
            path = SanitizeFileName(path);

            // If it's an existing asset but the path doesn't match, make a copy of it for the new path:
            if (AssetDatabase.Contains(asset)) {
                string assetPath = Path.GetFullPath(AssetDatabase.GetAssetPath(asset));
                string fullPath  = Path.GetFullPath(path);

                if (assetPath != fullPath) {
                    T newAsset = UnityEngine.Object.Instantiate(asset);
                    newAsset.name = asset.name;
                    asset         = newAsset;
                }
            }

            // to keep meta, rewrite the existing one if already exists.
            T loadedAsset = AssetDatabase.LoadAssetAtPath<T>(path);
            if (loadedAsset != null) {
                EditorUtility.CopySerialized(asset, loadedAsset);
                return loadedAsset;
            }

            AssetDatabase.CreateAsset(asset, path);
            return asset;
        }
        catch (Exception e) {
            Debug.LogError($"[MeshSync] Error in saving asset at {path}. Msg: {e.ToString()}");
            return null;
        }
    }

#endif

    internal static UnityEngine.TextureFormat ToUnityTextureFormat(TextureFormat v) {
        switch (v) {
            case TextureFormat.Ru8:     return UnityEngine.TextureFormat.R8;
            case TextureFormat.RGu8:    return UnityEngine.TextureFormat.RG16;
            case TextureFormat.RGBu8:   return UnityEngine.TextureFormat.RGB24;
            case TextureFormat.RGBAu8:  return UnityEngine.TextureFormat.RGBA32;
            case TextureFormat.Rf16:    return UnityEngine.TextureFormat.RHalf;
            case TextureFormat.RGf16:   return UnityEngine.TextureFormat.RGHalf;
            case TextureFormat.RGBAf16: return UnityEngine.TextureFormat.RGBAHalf;
            case TextureFormat.Rf32:    return UnityEngine.TextureFormat.RFloat;
            case TextureFormat.RGf32:   return UnityEngine.TextureFormat.RGFloat;
            case TextureFormat.RGBAf32: return UnityEngine.TextureFormat.RGBAFloat;
            default:                    return UnityEngine.TextureFormat.Alpha8;
        }
    }

    internal static TextureFormat ToMSTextureFormat(UnityEngine.TextureFormat v) {
        switch (v) {
            case UnityEngine.TextureFormat.R8:        return TextureFormat.Ru8;
            case UnityEngine.TextureFormat.RG16:      return TextureFormat.RGu8;
            case UnityEngine.TextureFormat.RGB24:     return TextureFormat.RGBu8;
            case UnityEngine.TextureFormat.RGBA32:    return TextureFormat.RGBAu8;
            case UnityEngine.TextureFormat.RHalf:     return TextureFormat.Rf16;
            case UnityEngine.TextureFormat.RGHalf:    return TextureFormat.RGf16;
            case UnityEngine.TextureFormat.RGBAHalf:  return TextureFormat.RGBAf16;
            case UnityEngine.TextureFormat.RFloat:    return TextureFormat.Rf32;
            case UnityEngine.TextureFormat.RGFloat:   return TextureFormat.RGf32;
            case UnityEngine.TextureFormat.RGBAFloat: return TextureFormat.RGBAf32;
            default:                                  return TextureFormat.Ru8;
        }
    }

    // explicit layout doesn't work with generics...
    [StructLayout(LayoutKind.Explicit)]
    private struct NAByte {
        [FieldOffset(0)] public NativeArray<byte> nativeArray;
        [FieldOffset(0)] public IntPtr            pointer;
    }

    public static IntPtr ForceGetPointer(ref NativeArray<byte> na) {
        NAByte union = new NAByte();
        union.nativeArray = na;
        return union.pointer;
    }

    [StructLayout(LayoutKind.Explicit)]
    private struct NABoneWeight1 {
        [FieldOffset(0)] public NativeArray<BoneWeight1> nativeArray;
        [FieldOffset(0)] public IntPtr                   pointer;
    }

    public static IntPtr ForceGetPointer(ref NativeArray<BoneWeight1> na) {
        NABoneWeight1 union = new NABoneWeight1();
        union.nativeArray = na;
        return union.pointer;
    }

    internal class UniqueNameGenerator {
        public string Gen(string name) {
            string uniqueName = name;
            for (int i = 1;; ++i)
                if (m_usedNames.Contains(uniqueName)) {
                    uniqueName = string.Format("{0} ({1})", name, i);
                }
                else {
                    m_usedNames.Add(uniqueName);
                    break;
                }

            return uniqueName;
        }

        private HashSet<string> m_usedNames = new HashSet<string>();
    }
}
} //end namespace