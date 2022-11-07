﻿using UnityEngine;

namespace Unity.MeshSync {
internal static class MaterialExtensions {
    /// <summary>
    /// Sets the given texture on the material.
    /// If there was a render texture in the slot, it is released.
    /// </summary>
    /// <param name="mat">The material to set the texture on</param>
    /// <param name="name">The name of the texture slot</param>
    /// <param name="texture">The texture to set</param>
    public  static void SetTextureSafe(this Material mat, int nameID, Texture texture) {
        // If there was a renderTexture set that we're clearing now, make sure to release its resources:
        var existingTexture = mat.GetTexture(nameID);
        if (existingTexture is RenderTexture existingRenderTexture &&
            texture != existingRenderTexture) {
            existingRenderTexture.Release();
        }

        mat.SetTexture(nameID, texture);
    }
}
}