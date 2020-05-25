using System;


namespace UnityEditor.MeshSync {

[Serializable]
internal class AnimationTweakEditorSettings {

    internal AnimationTweakEditorSettings() {
        
    }
//----------------------------------------------------------------------------------------------------------------------    
    internal AnimationTweakEditorSettings(AnimationTweakEditorSettings other) {
        
        AnimationFrameRate  = other.AnimationFrameRate;
        AnimationTimeScale  = other.AnimationTimeScale;
        AnimationTimeOffset = other.AnimationTimeOffset;
        AnimationDropStep   = other.AnimationDropStep;
        ReductionThreshold  = other.ReductionThreshold;
        EraseFlatCurves     = other.EraseFlatCurves;
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    public float AnimationFrameRate  = 30.0f;
    public float AnimationTimeScale  = 1.0f;
    public float AnimationTimeOffset = 0.0f;
    public int   AnimationDropStep   = 2;
    public float ReductionThreshold  = 0.001f;
    public bool  EraseFlatCurves     = false;

}

} //end namespace