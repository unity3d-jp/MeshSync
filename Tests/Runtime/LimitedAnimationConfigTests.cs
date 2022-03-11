using NUnit.Framework;

namespace Unity.MeshSync.Tests {

internal class LimitedAnimationConfigTests {
   
//----------------------------------------------------------------------------------------------------------------------    
    [Test]
    public void CheckLimitedAnimation() {

        LimitedAnimationConfig config = new LimitedAnimationConfig(true, numFramesToHold:3, frameOffset:0);        
        Assert.AreEqual(0, config.Apply(1));
        Assert.AreEqual(0, config.Apply(2));
        Assert.AreEqual(3, config.Apply(3));
        
        config.SetFrameOffset(2);
        Assert.AreEqual(2, config.Apply(1));
        Assert.AreEqual(2, config.Apply(2));
        Assert.AreEqual(2, config.Apply(3));
        Assert.AreEqual(2, config.Apply(4));
        Assert.AreEqual(5, config.Apply(5));
    }    


    
}

} //end namespace
