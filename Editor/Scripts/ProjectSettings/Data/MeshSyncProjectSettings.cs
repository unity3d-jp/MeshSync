using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;


namespace UnityEditor.MeshSync {

[Serializable]
internal class MeshSyncProjectSettings : ISerializationCallbackReceiver{

    internal static MeshSyncProjectSettings GetOrCreateSettings() {
        MeshSyncProjectSettings settings = LoadProjectSettings();
        if (null != settings) {
            return settings;
        }

        settings = new MeshSyncProjectSettings();
        settings.AddInstalledDCCTools();
        settings.SaveProjectSettings();
        return settings;
            
    }
//----------------------------------------------------------------------------------------------------------------------

    //Constructor
    MeshSyncProjectSettings() {
        m_dictionary = new SortedDictionary<string, DCCToolInfo>();
    }

//----------------------------------------------------------------------------------------------------------------------

    internal bool AddDCCToolInfo(string path, DCCToolType t, string version, bool save=true) {
        if (m_dictionary.ContainsKey(path))
            return false;

        DCCToolInfo newInfo = new DCCToolInfo(t, version) {
            AppPath = path
        };

        //
        m_dictionary.Add(path, newInfo);
        m_serializedDCCToolInfo.Add(newInfo);

        if (save) {
            SaveProjectSettings();
        }
        
        return true;
    }

//----------------------------------------------------------------------------------------------------------------------

    //return true if there is any change
    internal bool AddInstalledDCCTools() {

        bool ret = false;
        Dictionary<string, DCCToolInfo> dccPaths = ProjectSettingsUtility.FindInstalledDCCTools();
        foreach (var dcc in dccPaths) {
            DCCToolInfo info = dcc.Value;
            ret = ret || AddDCCToolInfo(dcc.Key, info.Type, info.Version, false);
        }

        return ret;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    #region File Load/Save for Serialization/deserialization
    static MeshSyncProjectSettings LoadProjectSettings() {
        string path = MESHSYNC_PROJECT_SETTINGS_PATH;
        if (!File.Exists(path)) {
            return null;
        }
        
        string json = File.ReadAllText(path);
        MeshSyncProjectSettings settings = JsonUtility.FromJson<MeshSyncProjectSettings>(json);
        return settings;
    }
    
    void SaveProjectSettings() {
        Directory.CreateDirectory(Path.GetDirectoryName(MESHSYNC_PROJECT_SETTINGS_PATH));
        string json = JsonUtility.ToJson(this);
        File.WriteAllText(MESHSYNC_PROJECT_SETTINGS_PATH, json);
        
    }
    #endregion
    

//----------------------------------------------------------------------------------------------------------------------

    #region ISerializationCallbackReceiver
    public void OnBeforeSerialize() {
        //[Note-sin: 2020-4-27] the list and dictionary should be already sync-end when in memory, 
        //so no need to do anything here
    }

    public void OnAfterDeserialize() {
        m_dictionary = new SortedDictionary<string, DCCToolInfo>();
        foreach (DCCToolInfo info in m_serializedDCCToolInfo) {
            if (m_dictionary.ContainsKey(info.AppPath)) {
                continue;
            }
            m_dictionary.Add(info.AppPath, info);
        }
    }
    #endregion

    
//----------------------------------------------------------------------------------------------------------------------

    internal IDictionary<string, DCCToolInfo> GetDCCToolInfos() { return m_dictionary; }
    
//----------------------------------------------------------------------------------------------------------------------


    const string MESHSYNC_PROJECT_SETTINGS_PATH = "Library/MeshSync/MeshSyncProjectSettings.asset";

    
    //Key: DCC Tool app path
    private SortedDictionary<string,DCCToolInfo> m_dictionary;
    [SerializeField] private List<DCCToolInfo> m_serializedDCCToolInfo = null;

    
}

} //end namespace