using System.Collections.Generic;
using UnityEngine;

namespace Unity.MeshSync {
    /// <summary>
    /// Holds meta data about the settings that were used to create the object.
    /// </summary>
    internal class MeshSyncServerLiveEditProperties : MonoBehaviour {
        [SerializeField]
        public List<PropertyInfoDataWrapper> propertyInfos = new List<PropertyInfoDataWrapper>();
    }
}
