using NUnit.Framework;
using UnityEditor;

namespace Unity.MeshSync.Editor.Tests {


public class SceneCachePlayerImporterTest  {

//----------------------------------------------------------------------------------------------------------------------
    [Test]
    public void CheckImporterProperties() {
        SceneCacheImporter importer         = new SceneCacheImporter();
        SerializedObject   serializedObject = new SerializedObject(importer);
        Assert.IsNotNull(serializedObject.FindProperty("m_importerSettings"));
    }
    
}

} //end namespace