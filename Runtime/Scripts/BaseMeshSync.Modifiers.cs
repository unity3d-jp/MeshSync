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
    [Serializable]
    public class PropertyInfoDataWrapper : ISerializationCallbackReceiver
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

        [field: SerializeField]
        public string name { get; private set; }

        [field: SerializeField]
        public float min { get; private set; }

        [field: SerializeField]
        public float max { get; private set; }

        [field: SerializeField]
        public string path { get; private set; }

        [field: SerializeField]
        public string modifierName { get; private set; }

        [field: SerializeField]
        public string propertyName { get; private set; }

        [field: SerializeField]
        public int arrayLength { get; private set; }

        // object cannot be serialized by unity, save a string representation of it:
        public object propertyValue;
        [SerializeField, HideInInspector]
        private string propertyValueSerialized;

        static T[] ParseArray<T>(string propertyValueSerialized, Func<string, T> parse)
        {
            string[] v = propertyValueSerialized.Substring(2).Split('|');
            var array = new T[v.Length];
            for (int i = 0; i < v.Length; i++)
            {
                array[i] = parse(v[i]);
            }

            return array;
        }

        static string SaveArray<T>(string prefix, T[] array)
        {
            StringBuilder sb = new StringBuilder(prefix);
            for (int i = 0; i < array.Length; i++)
            {
                if (i < array.Length)
                {
                    sb.Append("|");
                }

                sb.Append(array[i]);
            }

            return sb.ToString();
        }

        public string GetSerializedValue(bool useNewValues = false)
        {
            var valueToSerialize = propertyValue;

            if (useNewValues && newValue != null)
            {
                valueToSerialize = newValue;
            }

            if (valueToSerialize == null)
                return "n";
            else if (valueToSerialize is int)
                return "i" + valueToSerialize.ToString();
            else if (valueToSerialize is float)
                return "f" + valueToSerialize.ToString();
            else if (valueToSerialize is string)
                return "s" + valueToSerialize.ToString();
            else if (valueToSerialize is int[] intArray)
                return SaveArray("a", intArray);
            else if (valueToSerialize is float[] floatArray)
                return SaveArray("b", floatArray);
            else
                throw new NotImplementedException($"propertyValue: {valueToSerialize.GetType()} cannot be serialized!");
        }

        public void SetSerializedValue(string serializedValue)
        {
            NewValue = DeserializeString(serializedValue);
        }

        static object DeserializeString(string serializedValue)
        {
            if (serializedValue.Length == 0)
                return null;
            char type = serializedValue[0];
            if (type == 'n')
                return null;
            else if (type == 'i')
                return int.Parse(serializedValue.Substring(1));
            else if (type == 'f')
                return float.Parse(serializedValue.Substring(1));
            else if (type == 's')
                return serializedValue.Substring(1);
            else if (type == 'a')
                return ParseArray(serializedValue, int.Parse);
            else if (type == 'b')
                return ParseArray(serializedValue, float.Parse);
            else
                throw new NotImplementedException($"propertyValue: {type} cannot be deserialized!");
        }

        public void OnBeforeSerialize()
        {
            propertyValueSerialized = GetSerializedValue();
        }

        public void OnAfterDeserialize()
        {
            propertyValue = DeserializeString(propertyValueSerialized);
        }

        public object NewValue
        {
            get => newValue;
            set
            {
                switch (type)
                {
                    case PropertyInfoData.Type.IntArray:
                        {
                            // TODO: Use custom IntVector here maybe:
                            var newValueAsArray = new int[arrayLength];
                            Vector3 newValueAsVector = (Vector3)value;

                            newValueAsArray[0] = (int)newValueAsVector.x;
                            newValueAsArray[1] = (int)newValueAsVector.y;
                            newValueAsArray[2] = (int)newValueAsVector.z;

                            value = newValueAsArray;
                            break;
                        }
                    case PropertyInfoData.Type.FloatArray:
                        {
                            var newValueAsArray = new float[arrayLength];
                            Vector3 newValueAsVector = (Vector3)value;

                            newValueAsArray[0] = newValueAsVector.x;
                            newValueAsArray[1] = newValueAsVector.y;
                            newValueAsArray[2] = newValueAsVector.z;

                            value = newValueAsArray;
                            break;
                        }
                }

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

            if (propertyValue == null)
            {
                return default(T);
            }

            return (T)propertyValue;
        }

        public override string ToString()
        {
            return $"PropertyInfoDataWrapper: {name}: {GetValue<object>()}";
        }
    }

    // Partial class for now to make merging code easier later.
    [Serializable]
    partial class BaseMeshSync
    {
        bool m_foldBlenderSettings;

        internal bool foldBlenderSettings
        {
            get { return m_foldBlenderSettings; }
            set { m_foldBlenderSettings = value; }
        }

        [SerializeField]
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
