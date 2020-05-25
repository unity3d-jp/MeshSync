using System;


namespace Unity.MeshSync {

[Serializable]
internal class AnimationTweakSettings {

    internal AnimationTweakSettings() {
        
    }
//----------------------------------------------------------------------------------------------------------------------    
    internal AnimationTweakSettings(AnimationTweakSettings other) {
        
        TimeScale  = other.TimeScale;
        TimeOffset = other.TimeOffset;
        DropStep   = other.DropStep;
        ReductionThreshold  = other.ReductionThreshold;
        EraseFlatCurves     = other.EraseFlatCurves;
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    public float TimeScale  = 1.0f;
    public float TimeOffset = 0.0f;
    public int   DropStep   = 2;
    public float ReductionThreshold  = 0.001f;
    public bool  EraseFlatCurves     = false;

}

} //end namespace