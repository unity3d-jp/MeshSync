using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;


namespace UnityEditor.MeshSync {

[Serializable]
internal class MeshSyncProjectSettings {

    internal static MeshSyncProjectSettings GetOrCreateSettings() {
        string path = MESHSYNC_PROJECT_SETTINGS_PATH;
        MeshSyncProjectSettings settings = null;
        if (File.Exists(path)) {
            string json = File.ReadAllText(path);
            settings = JsonUtility.FromJson<MeshSyncProjectSettings>(json);
        }
        
        if (null == settings) {
            
            settings = new MeshSyncProjectSettings();
            settings.m_dccToolInfoList = new List<DCCToolInfo>();
            
            
            //[TODO-sin:2020-4-20] Remove temporary data
            settings.AddDCCToolInfo(DCCToolType.AUTODESK_MAYA, "2017");
            settings.AddDCCToolInfo(DCCToolType.AUTODESK_MAYA, "2018");
            settings.AddDCCToolInfo(DCCToolType.AUTODESK_MAYA, "2019");
            settings.AddDCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2019");
            settings.AddDCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2020");

            settings.Save();
            
        }
        return settings;
    }

//----------------------------------------------------------------------------------------------------------------------

    internal bool AddDCCToolInfo(DCCToolType t, string version) {
        m_dccToolInfoList.Add(new DCCToolInfo() {
            Type = t,
            Version = version,
        });

        Save();
        
        //[TODO-sin: 2020-4-20] Should return if successfully added or not
        return true;
    }

    void Save() {
        Directory.CreateDirectory(Path.GetDirectoryName(MESHSYNC_PROJECT_SETTINGS_PATH));
        string json = JsonUtility.ToJson(this);
        File.WriteAllText(MESHSYNC_PROJECT_SETTINGS_PATH, json);
        
    }
//----------------------------------------------------------------------------------------------------------------------

    internal IList<DCCToolInfo> GetDCCToolInfos() { return m_dccToolInfoList; }


//----------------------------------------------------------------------------------------------------------------------
    const string MESHSYNC_PROJECT_SETTINGS_PATH = "Library/MeshSync/MeshSyncProjectSettings.asset";

    
    //[TODO-sin: 2020-4-20] Use Dictionary and SerializerCallback
    [SerializeField] private List<DCCToolInfo> m_dccToolInfoList;
    
}

} //end namespace