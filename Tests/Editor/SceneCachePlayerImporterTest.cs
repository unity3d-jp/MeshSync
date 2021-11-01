using System.Reflection;
using NUnit.Framework;

namespace Unity.MeshSync.Editor.Tests {


public class SceneCachePlayerImporterTest  {

//----------------------------------------------------------------------------------------------------------------------
    [Test]
    public void CheckImporterProperties() {
        VerifyMemberExists(typeof(SceneCacheImporter), MeshSyncEditorConstants.SCENE_CACHE_IMPORTER_SETTINGS_PROP);
    }


    static void VerifyMemberExists(System.Type type, string memberName) {
        MemberInfo[] m = type.GetMember(memberName, BindingFlags.NonPublic | BindingFlags.Instance);
        Assert.Greater(m.Length, 0);
        Assert.IsNotNull(m[0]);
        
    }
    
}

} //end namespace