using NUnit.Framework;

namespace Unity.MeshSync.Tests {

internal class LimitedAnimationControllerTests {
   
//----------------------------------------------------------------------------------------------------------------------    
    [Test]
    public void CheckLimitedAnimation() {

        LimitedAnimationController controller = new LimitedAnimationController(true, numFramesToHold:3, frameOffset:0);        
        Assert.AreEqual(0, controller.Apply(1));
        Assert.AreEqual(0, controller.Apply(2));
        Assert.AreEqual(3, controller.Apply(3));
        
        controller.SetFrameOffset(2);
        Assert.AreEqual(2, controller.Apply(1));
        Assert.AreEqual(2, controller.Apply(2));
        Assert.AreEqual(2, controller.Apply(3));
        Assert.AreEqual(2, controller.Apply(4));
        Assert.AreEqual(5, controller.Apply(5));
    }    


    
}

} //end namespace
