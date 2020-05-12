using NUnit.Framework;
using UnityEngine;
using UnityEngine.TestTools;

namespace Unity.AnimeToolbox.Tests {
    internal class PathUtilityTest
    {
        [Test]
        [UnityPlatform(RuntimePlatform.OSXEditor)]
        public void GetDirectoryNamesOnOSX() {

            string dirName = null;
            dirName = PathUtility.TryGetDirectoryName("/Applications/Unity 2019/Unity.app", 1);
            Assert.AreEqual("/Applications/Unity 2019", dirName);
            dirName = PathUtility.TryGetDirectoryName("/Applications/Unity 2019/Unity.app/Contents/MacOS/Unity", 4);
            Assert.AreEqual("/Applications/Unity 2019", dirName);
            
            //Null checks
            dirName = PathUtility.TryGetDirectoryName("/Applications/Unity 2019/Unity.app", 4);
            Assert.IsNull(dirName);
            dirName = PathUtility.TryGetDirectoryName(null);
            Assert.IsNull(dirName);
            
        }
        
//----------------------------------------------------------------------------------------------------------------------

        [Test]
        [UnityPlatform(RuntimePlatform.WindowsEditor)]
        public void GetDirectoryNamesOnWindows() {

            string dirName = null;
            dirName = PathUtility.TryGetDirectoryName(@"C:\Program Files\Unity 2019\Unity.exe", 1);
            Assert.AreEqual(@"C:\Program Files\Unity 2019", dirName);
            dirName = PathUtility.TryGetDirectoryName(@"C:\Program Files\Unity 2019\Contents\Images", 3);
            Assert.AreEqual(@"C:\Program Files", dirName);
            
            //Null checks
            dirName = PathUtility.TryGetDirectoryName(@"C:\Program Files\Unity 2019\Unity.exe", 4);
            Assert.IsNull(dirName);
            dirName = PathUtility.TryGetDirectoryName(null);
            Assert.IsNull(dirName);
            
        }
    }
    
}
