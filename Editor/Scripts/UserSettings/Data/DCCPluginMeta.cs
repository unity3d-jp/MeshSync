using System;
using System.Collections.Generic;
using JetBrains.Annotations;
using UnityEngine;

namespace Unity.MeshSync.Editor {
    
[Serializable]
internal class DCCPluginMeta : ISerializationCallbackReceiver {

//----------------------------------------------------------------------------------------------------------------------    

    //May return null
    [CanBeNull]
    internal DCCPluginSignature GetSignature(string dccPluginFileName) {
        if (m_dictionary.ContainsKey(dccPluginFileName)) {
            return m_dictionary[dccPluginFileName];
        }

        return null;
    }
    
//----------------------------------------------------------------------------------------------------------------------    

    #region ISerializationCallbackReceiver

    /// <inheritdoc/>
    public void OnBeforeSerialize() {
        //Nothing. We don't support saving
    }

    /// <inheritdoc/>
    public void OnAfterDeserialize() {
        m_dictionary = new Dictionary<string, DCCPluginSignature>();
        foreach (DCCPluginSignature signature in FileSignatures) {
            if (m_dictionary.ContainsKey(signature.FileName)) {
                Debug.LogWarning("[MeshSync] Duplicate signature for " + signature.FileName);
                continue;
            }
            
            m_dictionary.Add(signature.FileName, signature);                        
        }        
    }
    #endregion

//----------------------------------------------------------------------------------------------------------------------    

    private Dictionary<string, DCCPluginSignature> m_dictionary;

    //Serialized fields
    [SerializeField] private List<DCCPluginSignature> FileSignatures = null;
    
    
}
    
} //end namespace
