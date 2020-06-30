using System;
using System.Collections.Generic;
using System.IO;
using Unity.MeshSync;
using UnityEngine;


namespace Unity.MeshSync.Editor {

[Serializable]
internal class MeshSyncEditorSettings : ISerializationCallbackReceiver{

    internal static MeshSyncEditorSettings GetOrCreateSettings() {
        MeshSyncEditorSettings settings = LoadEditorSettings();
        if (null != settings) {
            return settings;
        }

        settings = new MeshSyncEditorSettings();
        settings.AddInstalledDCCTools();
        settings.SaveEditorSettings();
        return settings;
            
    }

    internal static string GetSettingsPath() { return MESHSYNC_EDITOR_SETTINGS_PATH; }

//----------------------------------------------------------------------------------------------------------------------

    //Constructor
    MeshSyncEditorSettings() {
        m_serializedDCCToolInfo = new List<DCCToolInfo>();
        m_dictionary = new SortedDictionary<string, DCCToolInfo>();
    }

//----------------------------------------------------------------------------------------------------------------------

    internal bool AddDCCTool(DCCToolInfo dccToolInfo, bool save=true) {
        if (m_dictionary.ContainsKey(dccToolInfo.AppPath))
            return false;


        //
        m_dictionary.Add(dccToolInfo.AppPath, dccToolInfo);
        m_serializedDCCToolInfo.Add(dccToolInfo);

        if (save) {
            SaveEditorSettings();
        }
        
        return true;
    }

//----------------------------------------------------------------------------------------------------------------------
    internal bool RemoveDCCTool(string appPath) {
        if (!m_dictionary.ContainsKey(appPath)) {
            return false;
        }


        int numDCCTools = m_serializedDCCToolInfo.Count;
        int itemToRemove = numDCCTools;
        for (int i = 0; i < numDCCTools; ++i) {
            if (m_serializedDCCToolInfo[i].AppPath == appPath) {
                itemToRemove = i;
                break;
            }
        }

        if (itemToRemove >= numDCCTools) {
            Debug.LogError("[MeshSync] Internal error: " + appPath);
            return false;
        }

        m_serializedDCCToolInfo.RemoveAt(itemToRemove);
        m_dictionary.Remove(appPath);
        SaveEditorSettings();

        return true;
    }

//----------------------------------------------------------------------------------------------------------------------

    //return true if there is any change
    internal bool AddInstalledDCCTools() {

        bool ret = false;
        Dictionary<string, DCCToolInfo> dccPaths = DCCFinderUtility.FindInstalledDCCTools();
        foreach (var dcc in dccPaths) {
            DCCToolInfo info = dcc.Value;
            bool added = AddDCCTool(info, false);
            ret = ret || added;
        }

        SaveEditorSettings();
        return ret;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    #region File Load/Save for Serialization/deserialization
    static MeshSyncEditorSettings LoadEditorSettings() {
        string path = MESHSYNC_EDITOR_SETTINGS_PATH;
        if (!File.Exists(path)) {
            return null;
        }
        
        string json = File.ReadAllText(path);
        MeshSyncEditorSettings settings = JsonUtility.FromJson<MeshSyncEditorSettings>(json);
        
        return settings;
    }
    
    void SaveEditorSettings() {
        Directory.CreateDirectory(Path.GetDirectoryName(MESHSYNC_EDITOR_SETTINGS_PATH));
        string json = JsonUtility.ToJson(this);
        File.WriteAllText(MESHSYNC_EDITOR_SETTINGS_PATH, json);
        
    }
    #endregion
    

//----------------------------------------------------------------------------------------------------------------------

    #region ISerializationCallbackReceiver
    /// <inheritdoc/>
    public void OnBeforeSerialize() {
        //[Note-sin: 2020-4-27] the list and dictionary should be already sync-end when in memory, 
        //so no need to do anything here
    }

    /// <inheritdoc/>
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


    const string MESHSYNC_EDITOR_SETTINGS_PATH = "Library/MeshSync/MeshSyncEditorSettings.asset";

    
    //Key: DCC Tool app path
    private SortedDictionary<string,DCCToolInfo> m_dictionary;
    [SerializeField] private List<DCCToolInfo> m_serializedDCCToolInfo = null;

    [SerializeField] internal int ClassVersion = 1;

}

} //end namespace