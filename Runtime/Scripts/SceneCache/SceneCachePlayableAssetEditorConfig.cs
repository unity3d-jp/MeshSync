using UnityEngine;

namespace Unity.MeshSync {

[System.Serializable] 
internal class SceneCachePlayableAssetEditorConfig {
    
    internal void SetGenerateStartKeyFrame(int frame) { m_generateStartKeyFrame = frame;}
    internal int  GetGenerateStartKeyFrame()          { return m_generateStartKeyFrame;}
    internal void SetGenerateEndKeyFrame(int frame)   { m_generateEndKeyFrame = frame;}
    internal int  GetGenerateEndKeyFrame()            { return m_generateEndKeyFrame;}

    internal void SetGenerateAllKeyFrames(bool generate) { m_generateAllKeyFrames = generate;}
    internal bool GetGenerateAllKeyFrames()             { return m_generateAllKeyFrames;}

    internal int  GetGenerateKeyFrameSpan()         => m_generateKeyFrameSpan;
    internal void SetGenerateKeyFrameSpan(int span) { m_generateKeyFrameSpan = Mathf.Max(1,span); }
    
    internal KeyFrameMode GetGenerateKeyFrameMode()                  => m_generateKeyFrameMode;
    internal void         SetGenerateKeyFrameMode(KeyFrameMode mode) { m_generateKeyFrameMode = mode; } 
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] [HideInInspector] private int          m_generateKeyFrameSpan = 3;
    [SerializeField] [HideInInspector] private KeyFrameMode m_generateKeyFrameMode = KeyFrameMode.Continuous;
    
    [SerializeField] [HideInInspector] private int  m_generateStartKeyFrame = 0;
    [SerializeField][HideInInspector]  private int  m_generateEndKeyFrame   = -1;
    [SerializeField][HideInInspector]  private bool m_generateAllKeyFrames  = true;

}


}