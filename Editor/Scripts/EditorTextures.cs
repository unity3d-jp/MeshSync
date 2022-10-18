using Unity.FilmInternalUtilities;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {
internal static class EditorTextures {
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    internal static Texture GetKeyFrameSmoothTexture() {
        if (!m_keyFrameSmoothTexture.IsNullRef()) {
            return m_keyFrameSmoothTexture;
        }


        m_keyFrameSmoothTexture = (Texture2D)EditorGUIUtility.Load("d_PlayButton On@2x");
        return m_keyFrameSmoothTexture;
    }

    internal static Texture GetKeyFrameStopTexture() {
        if (!m_keyFrameStopTexture.IsNullRef()) {
            return m_keyFrameStopTexture;
        }

        m_keyFrameStopTexture = (Texture2D)EditorGUIUtility.Load("d_PauseButton@2x");
        return m_keyFrameStopTexture;
    }


//----------------------------------------------------------------------------------------------------------------------

    private static Texture m_keyFrameSmoothTexture = null;
    private static Texture m_keyFrameStopTexture   = null;
}
} //end namespace