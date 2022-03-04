using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Unity.MeshSync
{
    partial class MeshSyncServer
    {
        void SendUpdatedProperties()
        {
            foreach (var prop in propertyInfos)
            {
                if (prop.IsDirty)
                {
                    m_server.SendProperty(prop);
                    prop.IsDirty = false;
                }
            }
        }
    }
}
