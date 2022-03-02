using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Unity.MeshSync;
using UnityEngine;

namespace Unity.MeshSync
{
    // Partial class for now to make merging code easier later.
    partial class BaseMeshSync
    {
        internal List<PropertyInfoData> propertyInfos = new List<PropertyInfoData>();

        void UpdateProperties(SceneData scene)
        {
            // handle properties
            Try(() =>
            {
                propertyInfos.Clear();
                var numProperties = scene.numPropertyInfos;
                for (var i = 0; i < numProperties; ++i)
                {
                    var data = scene.GetPropertyInfo(i);
                    propertyInfos.Add(data);
                }
            });
        }
    }
}
