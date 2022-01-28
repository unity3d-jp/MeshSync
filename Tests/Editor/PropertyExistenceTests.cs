using System.Reflection;
using NUnit.Framework;

namespace Unity.MeshSync.Editor.Tests {


internal class PropertyExistenceTests  {

//----------------------------------------------------------------------------------------------------------------------
    [Test]
    public void CheckSceneCacheImporterProperties() {
        VerifyMemberExists(typeof(SceneCacheImporter), SceneCacheImporter.IMPORTER_SETTINGS_PROP);
    }

    [Test]
    public void CheckModelImporterSettingsProperties() {
        VerifyMemberExists(typeof(ModelImporterSettings), ModelImporterSettings.CREATE_MATERIALS_PROP);
        VerifyMemberExists(typeof(ModelImporterSettings), ModelImporterSettings.MATERIAL_SEARCH_MODE_PROP);
    }

    
    static void VerifyMemberExists(System.Type type, string memberName) {
        MemberInfo[] m = type.GetMember(memberName, BindingFlags.NonPublic | BindingFlags.Instance);
        Assert.Greater(m.Length, 0);
        Assert.IsNotNull(m[0]);
        
    }
    
}

} //end namespace