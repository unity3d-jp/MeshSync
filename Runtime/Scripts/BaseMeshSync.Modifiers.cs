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
    public class PropertyInfoDataWrapper
    {
        [DllImport(Lib.name)]
        static extern void msPropertyInfoCopyData(IntPtr self, ref int dst);

        [DllImport(Lib.name)]
        static extern void msPropertyInfoCopyData(IntPtr self, ref float dst);

        [DllImport(Lib.name)]
        static extern void msPropertyInfoCopyData(IntPtr self, StringBuilder dst);

        [DllImport(Lib.name)]
        static extern void msPropertyInfoCopyData(IntPtr self, float[] dst);

        [DllImport(Lib.name)]
        static extern void msPropertyInfoCopyData(IntPtr self, int[] dst);

        [DllImport(Lib.name)]
        static extern int msPropertyInfoGetArrayLength(IntPtr self);

        public PropertyInfoDataWrapper(PropertyInfoData propertyInfoData)
        {
            type = propertyInfoData.type;
            sourceType = propertyInfoData.sourceType;
            name = propertyInfoData.name;
            min = propertyInfoData.min;
            max = propertyInfoData.max;
            path = propertyInfoData.path;
            modifierName = propertyInfoData.modifierName;
            propertyName = propertyInfoData.propertyName;

            arrayLength = msPropertyInfoGetArrayLength(propertyInfoData.self);

            switch (type)
            {
                case PropertyInfoData.Type.Int:
                    {
                        int r = 0;
                        msPropertyInfoCopyData(propertyInfoData.self, ref r);
                        propertyValue = r;
                        break;
                    }
                case PropertyInfoData.Type.Float:
                    {
                        float r = 0;
                        msPropertyInfoCopyData(propertyInfoData.self, ref r);
                        propertyValue = r;
                        break;
                    }
                case PropertyInfoData.Type.IntArray:
                    {
                        int[] r = new int[arrayLength];
                        msPropertyInfoCopyData(propertyInfoData.self, r);
                        propertyValue = r;
                        break;
                    }
                case PropertyInfoData.Type.FloatArray:
                    {
                        float[] r = new float[arrayLength];
                        msPropertyInfoCopyData(propertyInfoData.self, r);
                        propertyValue = r;
                        break;
                    }

                case PropertyInfoData.Type.String:
                    {
                        var s = new StringBuilder(arrayLength + 1); // +1 for string terminator
                        msPropertyInfoCopyData(propertyInfoData.self, s);
                        propertyValue = s.ToString();
                        break;
                    }

                default:
                    Debug.LogError($"Type {propertyInfoData.type} not implemented");
                    break;
            }
        }

        public PropertyInfoData.Type type;
        public PropertyInfoData.SourceType sourceType;

        public string name { get; private set; }

        public float min { get; private set; }
        public float max { get; private set; }
        public string path { get; private set; }

        public string modifierName { get; private set; }

        public string propertyName { get; private set; }

        public int arrayLength { get; private set; }

        private object propertyValue;

        public object NewValue
        {
            get => newValue;
            set
            {
                if (newValue != null)
                {
                    if (newValue.Equals(value))
                    {
                        return;
                    }
                }
                else if (propertyValue != null && propertyValue.Equals(value))
                {
                    return;
                }

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

        public T GetValue<T>()
        {
            if (NewValue is T x)
            {
                return x;
            }

            return (T)propertyValue;
        }
    }

    // Partial class for now to make merging code easier later.
    partial class BaseMeshSync
    {
        bool m_foldBlenderSettings;

        internal bool foldBlenderSettings
        {
            get { return m_foldBlenderSettings; }
            set { m_foldBlenderSettings = value; }
        }

        public List<PropertyInfoDataWrapper> propertyInfos = new List<PropertyInfoDataWrapper>();

        void UpdateProperties(SceneData scene)
        {
            // handle properties
            Try(() =>
            {
                var numProperties = scene.numPropertyInfos;
                if (numProperties > 0)
                {
                    propertyInfos.Clear();
                    for (var i = 0; i < numProperties; ++i)
                    {
                        var data = scene.GetPropertyInfo(i);
                        propertyInfos.Add(new PropertyInfoDataWrapper(data));
                    }
                }
            });
        }
    }
}
