using Unity.FilmInternalUtilities;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {
internal static class EditorTextures {
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    internal static Texture GetKeyFrameSmoothTexture() {
        return GetOrLoadTexture(ref m_keyFrameSmoothTexture, "d_PlayButton On@2x");
    }

    internal static Texture GetKeyFrameStopTexture() {
        return GetOrLoadTexture(ref m_keyFrameStopTexture, "d_PlayButtonProfile On");
        
    }
    internal static Texture GetTextBackgroundTexture() {
        return GetOrLoadTexture(ref m_textBackGroundTexture, "gameviewbackground");
    }

    static Texture GetOrLoadTexture(ref Texture tex, string texPath) {
        if (!tex.IsNullRef()) {
            return tex;
        }

        tex = (Texture2D)EditorGUIUtility.Load(texPath);
        return tex;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    private static Texture m_keyFrameSmoothTexture = null;
    private static Texture m_keyFrameStopTexture   = null;
    private static Texture m_textBackGroundTexture = null;
}
} //end namespace