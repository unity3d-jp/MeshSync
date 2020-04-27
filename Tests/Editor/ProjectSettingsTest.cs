using System.Collections.Generic;
using NUnit.Framework;

namespace UnityEditor.MeshSync.Tests {
internal class ProjectSettingsTest {
    
    
    [Test]
    public void CheckInstalledDCCTools() {

        Dictionary<string, DCCToolInfo> dccToolInfos = ProjectSettingsUtility.FindInstalledDCCTools();
        Assert.GreaterOrEqual(dccToolInfos.Count,0);
    }    
    
//----------------------------------------------------------------------------------------------------------------------    
    [Test]
    public void CheckCurrentSettings() {
        MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
        Assert.NotNull(settings);
    }    
    
}

} //end namespace
