using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Unity.MeshSync;
using UnityEngine;

namespace Unity.MeshSync
{
    // Wrapper with additional data on PropertyInfoData
    // that cannot be stored in the shared data structure:
    class PropertyInfoDataWrapper
    {
        [DllImport(Lib.name)]
        static extern void msPropertyInfoCopyData(IntPtr self, ref int dst);

        [DllImport(Lib.name)]
        static extern void msPropertyInfoCopyData(IntPtr self, ref float dst);


        public PropertyInfoDataWrapper(PropertyInfoData propertyInfoData)
        {
            this.propertyInfoData = propertyInfoData;
        }

        private PropertyInfoData propertyInfoData;

        public PropertyInfoData.Type type => propertyInfoData.type;

        public IntPtr propertyPointer => propertyInfoData.self;

        public string name => propertyInfoData.name;
        public float min => propertyInfoData.min;
        public float max => propertyInfoData.max;
        public string path => propertyInfoData.path;


        public object NewValue
        {
            get => newValue; set
            {
                newValue = value;
                IsDirty = true;
            }
        }

        private object newValue;

        public bool IsDirty
        {
            get;
            set;
        }

        public int ValueInt
        {
            get
            {
                if (NewValue is int x)
                {
                    return x;
                }

                int r = 0;
                msPropertyInfoCopyData(propertyInfoData.self, ref r);
                return r;
            }
        }

        public float ValueFloat
        {
            get
            {
                if (NewValue is float x)
                {
                    return x;
                }

                float r = 0;
                msPropertyInfoCopyData(propertyInfoData.self, ref r);
                return r;
            }
        }
    }

    // Partial class for now to make merging code easier later.
    partial class BaseMeshSync
    {
        internal List<PropertyInfoDataWrapper> propertyInfos = new List<PropertyInfoDataWrapper>();

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
                    propertyInfos.Add(new PropertyInfoDataWrapper(data));
                }
            });

        }
    }
}
