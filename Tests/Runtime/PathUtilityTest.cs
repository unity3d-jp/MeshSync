using NUnit.Framework;

namespace Unity.AnimeToolbox.Tests {
    internal class PathUtilityTest
    {

        [Test]
        public void GetDirectoryNames() {

            string dirName = null;
            dirName = PathUtility.TryGetDirectoryName("/Applications/Unity 2019/Unity.app", 1);
            Assert.AreEqual(dirName, "/Applications/Unity 2019");
            dirName = PathUtility.TryGetDirectoryName("/Applications/Unity 2019/Unity.app/Contents/MacOS/Unity", 4);
            Assert.AreEqual(dirName, "/Applications/Unity 2019");
            
            //Null checks
            dirName = PathUtility.TryGetDirectoryName("/Applications/Unity 2019/Unity.app", 4);
            Assert.IsNull(dirName);
            dirName = PathUtility.TryGetDirectoryName(null);
            Assert.IsNull(dirName);
            
        }

    }
    
}
