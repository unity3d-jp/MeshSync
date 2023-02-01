using System.Collections.Generic;
using System.Linq;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync {
/// <summary>
/// Helper class to bake smoothness and metallic into channels required for the standard shaders.
/// </summary>
internal static class MapsBaker {
    private const string _SmoothnessTextureChannel = "_SmoothnessTextureChannel";

    private static Dictionary<string, ComputeShaderHelper> loadedShaders = new Dictionary<string, ComputeShaderHelper>();

    private const string SHADER_CONST_METALLIC   = "Metallic";
    private const string SHADER_CONST_SMOOTHNESS = "Smoothness";
    private const string SHADER_CONST_RGB        = "RGB";

    private const string SHADER_FILE                       = "meshsync_channel_mapping";
    private const string SHADER_NAME_SMOOTHNESS_INTO_ALPHA = "smoothness_into_alpha";
    private const string SHADER_NAME_ROUGHNESS_INTO_ALPHA  = "roughness_into_alpha";
    private const string SHADER_NAME_HDRP_MASK             = "hdrp_mask";
    private const string SHADER_NAME_HDRP_MASK_ROUGHNESS   = "hdrp_mask_roughness";


    private static ComputeShaderHelper LoadShader(string name) {
        string file = SHADER_FILE;

        if (!loadedShaders.TryGetValue(file, out ComputeShaderHelper shaderHelper)) {
#if UNITY_EDITOR
            string[] shaderFiles = AssetDatabase.FindAssets(file);
            if (shaderFiles.Length > 0) {
                string        shaderFile = shaderFiles[0];
                ComputeShader shader     = AssetDatabase.LoadAssetAtPath<ComputeShader>(AssetDatabase.GUIDToAssetPath(shaderFile));

                shaderHelper = new ComputeShaderHelper(shader, name);

                loadedShaders[name] = shaderHelper;
            }
#else
// TODO (BLENDER-614): Load shader at runtime here
#endif
        }

        return shaderHelper;
    }

    private static bool FindTexture(int nameID,
        List<TextureHolder> textureHolders,
        Dictionary<int, IMaterialPropertyData> materialProperties,
        int fallbackPropertyNameID,
        float fallbackPropertyValue,
        out Texture2DDisposable disposableTexture) {
        if (materialProperties.TryGetValue(nameID, out IMaterialPropertyData prop)) {
            IMaterialPropertyData.TextureRecord rec = prop.textureValue;
            disposableTexture = new Texture2DDisposable(BaseMeshSync.FindTexture(rec.id, textureHolders), false);

            if (disposableTexture.Texture != null) return true;
        }

        // If there is no such texture, create one with the given fallback value:

        float value = fallbackPropertyValue;
        if (materialProperties.TryGetValue(fallbackPropertyNameID, out IMaterialPropertyData fallbackMaterialProperty))
            value = fallbackMaterialProperty.floatValue;

        const int dim = 8;

        Color[] pixels = Enumerable.Repeat(new Color(value, value, value, value), dim * dim).ToArray();

        disposableTexture =
            new Texture2DDisposable(new Texture2D(dim, dim, UnityEngine.TextureFormat.RFloat, false, true));

        disposableTexture.Texture.SetPixels(pixels);
        disposableTexture.Texture.Apply();

        return false;
    }

    private static void GetSmoothnessOrRoughnessMap(
        Dictionary<int, IMaterialPropertyData> materialProperties,
        List<TextureHolder> textureHolders,
        out Texture2DDisposable texture,
        out bool usingRoughness,
        ref bool texturesExist) {
        // If there is a roughness map, use that instead of smoothness and invert it:
        if (materialProperties.ContainsKey(MeshSyncConstants._RoughMap)) {
            usingRoughness = true;

            texturesExist |=
                FindTexture(MeshSyncConstants._RoughMap, textureHolders, materialProperties, 0, 0,
                    out texture);
        }
        else {
            usingRoughness = false;
            // Bake smoothness to 1 if there is no map and use the slider to scale it:
            texturesExist |=
                FindTexture(MeshSyncConstants._GlossMap, textureHolders, materialProperties,
                    MeshSyncConstants._Glossiness, 1, out texture);
        }
    }

#if AT_USE_HDRP
    private static void BakeMaskMap(Material destMat,
        List<TextureHolder> textureHolders,
        Dictionary<int, IMaterialPropertyData> materialProperties) {
        if (!destMat.HasProperty(MeshSyncConstants._MaskMap)) return;

        // Mask map:
        // R: metal
        // G: ao
        // B: detail mask
        // A: smoothness

        bool texturesExist = false;

        texturesExist |=
            FindTexture(MeshSyncConstants._MetallicGlossMap, textureHolders, materialProperties,
                MeshSyncConstants._Metallic, 0, out Texture2DDisposable metalTexture);

        GetSmoothnessOrRoughnessMap(
            materialProperties,
            textureHolders,
            out Texture2DDisposable glossOrRoughTexture,
            out bool usingRoughness,
            ref texturesExist);

        // If there are no textures, don't bake anything, slider values can control everything:
        if (!texturesExist) {
            destMat.SetTextureAndReleaseExistingRenderTextures(MeshSyncConstants._MaskMap, null);
            destMat.DisableKeyword(MeshSyncConstants._MASKMAP);
            return;
        }

        ComputeShaderHelper shader;
        if (usingRoughness)
            shader = LoadShader(SHADER_NAME_HDRP_MASK_ROUGHNESS);
        else
            shader = LoadShader(SHADER_NAME_HDRP_MASK);

        if (shader == null) return;

        shader.SetTexture(SHADER_CONST_METALLIC, metalTexture.Texture);
        shader.SetTexture(SHADER_CONST_SMOOTHNESS, glossOrRoughTexture.Texture);

        RenderTexture texture = shader.RenderToTexture(destMat.GetTexture(MeshSyncConstants._MaskMap));

        destMat.EnableKeyword(MeshSyncConstants._MASKMAP);

        destMat.SetTextureAndReleaseExistingRenderTextures(MeshSyncConstants._MaskMap, texture);

        metalTexture.Dispose();
        glossOrRoughTexture.Dispose();
    }
#else
    private static void BakeSmoothness(Material destMat,
        List<TextureHolder> textureHolders,
        Dictionary<int, IMaterialPropertyData> materialProperties) {
        if (!destMat.HasProperty(_SmoothnessTextureChannel)) return;

        int smoothnessChannel = destMat.GetInt(_SmoothnessTextureChannel);

        Texture2DDisposable rgbTexture;
        int                 channelName;

        bool texturesExist = false;

        if (smoothnessChannel == 0) {
            // Bake to metallic alpha
            channelName = MeshSyncConstants._MetallicGlossMap;
            texturesExist |=
                FindTexture(MeshSyncConstants._MetallicGlossMap, textureHolders, materialProperties,
                    MeshSyncConstants._Metallic, 0, out rgbTexture);
        }
        else if (smoothnessChannel == 1) {
            // Bake to albedo alpha
            channelName = MeshSyncConstants._MainTex;
            texturesExist |=
                FindTexture(MeshSyncConstants._MainTex, textureHolders, materialProperties, 0, 0,
                    out rgbTexture);
        }
        else {
            Debug.LogError(
                $"[MeshSync] Unknown smoothness channel, cannot bake to option: {smoothnessChannel} set in the smoothness texture channel.");
            return;
        }

        GetSmoothnessOrRoughnessMap(
            materialProperties,
            textureHolders,
            out Texture2DDisposable glossOrRoughTexture,
            out bool usingRoughness,
            ref texturesExist);

        // If there are no textures, don't bake anything, slider values can control everything:
        if (!texturesExist) {
            destMat.SetTextureAndReleaseExistingRenderTextures(channelName, null);

            if (channelName == MeshSyncConstants._MetallicGlossMap) {
                destMat.DisableKeyword(MeshSyncConstants._METALLICGLOSSMAP);
                destMat.DisableKeyword(MeshSyncConstants._METALLICSPECGLOSSMAP);
            }

            return;
        }

        ComputeShaderHelper shader;
        if (usingRoughness)
            shader = LoadShader(SHADER_NAME_ROUGHNESS_INTO_ALPHA);
        else
            shader = LoadShader(SHADER_NAME_SMOOTHNESS_INTO_ALPHA);

        if (shader == null) return;

        shader.SetTexture(SHADER_CONST_SMOOTHNESS, glossOrRoughTexture.Texture);
        shader.SetTexture(SHADER_CONST_RGB, rgbTexture.Texture);

        RenderTexture texture = shader.RenderToTexture(destMat.GetTexture(channelName));

        if (channelName == MeshSyncConstants._MetallicGlossMap) {
            destMat.EnableKeyword(MeshSyncConstants._METALLICGLOSSMAP);
            destMat.EnableKeyword(MeshSyncConstants._METALLICSPECGLOSSMAP);
        }
        else if (smoothnessChannel == 0) {
            destMat.DisableKeyword(MeshSyncConstants._METALLICGLOSSMAP);
            destMat.DisableKeyword(MeshSyncConstants._METALLICSPECGLOSSMAP);
        }

        destMat.SetTextureAndReleaseExistingRenderTextures(channelName, texture);

        rgbTexture.Dispose();
        glossOrRoughTexture.Dispose();
    }
#endif

    public static void BakeMaps(Material destMat,
        List<TextureHolder> textureHolders,
        Dictionary<int, IMaterialPropertyData> materialProperties) {
#if AT_USE_HDRP
        BakeMaskMap(destMat, textureHolders, materialProperties);
#else
        BakeSmoothness(destMat, textureHolders, materialProperties);
#endif
    }
}
}