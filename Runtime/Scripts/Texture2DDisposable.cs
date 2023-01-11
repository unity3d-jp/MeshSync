using System;
using UnityEngine;
using Object = UnityEngine.Object;

namespace Unity.MeshSync {
/// <summary>
/// Wrapper class for Texture2D to handle cleaning up resources when the texture is
/// no longer required.
/// </summary>
internal class Texture2DDisposable : IDisposable {
    public Texture2D Texture { get; private set; }

    private bool DoDispose { get; }

    public Texture2DDisposable(Texture2D texture, bool doDispose = true) {
        Init(texture);
        DoDispose = doDispose;
    }

    private void Init(Texture2D texture) {
        Texture = texture;
    }

    public void ReadFromRenderTexture(RenderTexture renderTexture) {
        RenderTexture activeRenderTarget = RenderTexture.active;

        RenderTexture.active = renderTexture;
        Texture.ReadPixels(new Rect(0, 0, renderTexture.width, renderTexture.height), 0, 0);
        Texture.Apply();

        RenderTexture.active = activeRenderTarget;
    }

    public void Dispose() {
        if (!DoDispose) return;

        if (Application.isPlaying)
            Object.Destroy(Texture);
        else
            Object.DestroyImmediate(Texture);
    }
}
}