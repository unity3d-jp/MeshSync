using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace Unity.MeshSync
{
    /// <summary>
    /// Holds meta data about the settings that were used to create the object.
    /// </summary>
    internal class MeshSyncServerProperties : MonoBehaviour
    {
        [SerializeField]
        public List<PropertyInfoDataWrapper> propertyInfos = new List<PropertyInfoDataWrapper>();

        MeshSyncServer _server;

        public MeshSyncServer Server
        {
            get
            {
                if (_server == null)
                {
                    _server = GetComponent<MeshSyncServer>();
                }

                return _server;
            }
        }
    }
}
