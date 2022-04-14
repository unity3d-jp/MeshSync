using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace Unity.MeshSync
{
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
