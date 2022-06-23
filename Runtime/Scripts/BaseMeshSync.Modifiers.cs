using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Unity.MeshSync;
using UnityEngine;

#if AT_USE_SPLINES
using UnityEngine.Splines;
using Unity.Mathematics;
#endif

namespace Unity.MeshSync
{
    public class PropertyInfoDataWrapperComparer : IComparer<PropertyInfoDataWrapper>
    {
        public int Compare(PropertyInfoDataWrapper x, PropertyInfoDataWrapper y)
        {
            if (x.path == y.path)
            {
                return x.name.CompareTo(y.name);
            }

            return x.path.CompareTo(y.path);
        }
    }

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

        public static object PropertyUpdateLock = new object();

        internal PropertyInfoDataWrapper(PropertyInfoData propertyInfoData)
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
                case PropertyInfoDataType.Int:
                    {
                        int r = 0;
                        msPropertyInfoCopyData(propertyInfoData.self, ref r);
                        propertyValue = r;
                        break;
                    }
                case PropertyInfoDataType.Float:
                    {
                        float r = 0;
                        msPropertyInfoCopyData(propertyInfoData.self, ref r);
                        propertyValue = r;
                        break;
                    }
                case PropertyInfoDataType.IntArray:
                    {
                        int[] r = new int[arrayLength];
                        msPropertyInfoCopyData(propertyInfoData.self, r);
                        propertyValue = r;
                        break;
                    }
                case PropertyInfoDataType.FloatArray:
                    {
                        float[] r = new float[arrayLength];
                        msPropertyInfoCopyData(propertyInfoData.self, r);
                        propertyValue = r;
                        break;
                    }

                case PropertyInfoDataType.String:
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

        public PropertyInfoDataType type;
        public PropertyInfoDataSourceType sourceType;

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

        public string ID
        {
            get
            {
                return $"{path}_{name}";
            }
        }

        public bool Matches(PropertyInfoDataWrapper other)
        {
            return
                name == other.name &&
                type == other.type &&
                sourceType == other.sourceType &&
                propertyName == other.propertyName &&
                arrayLength == other.arrayLength;
        }

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

        public void SetSerializedValue(string serializedValue)
        {
            NewValue = DeserializeString(serializedValue);
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
                    case PropertyInfoDataType.IntArray:
                        {
                            var newValueAsArray = new int[arrayLength];

                            if (value is Vector2 newValueAsVector2)
                            {
                                newValueAsArray[0] = Mathf.Clamp((int)newValueAsVector2.x, (int)min, (int)max);
                                newValueAsArray[1] = Mathf.Clamp((int)newValueAsVector2.y, (int)min, (int)max);

                                value = newValueAsArray;
                            }
                            else if (value is Vector3 newValueAsVector3)
                            {
                                newValueAsArray[0] = Mathf.Clamp((int)newValueAsVector3.x, (int)min, (int)max);
                                newValueAsArray[1] = Mathf.Clamp((int)newValueAsVector3.y, (int)min, (int)max);
                                newValueAsArray[2] = Mathf.Clamp((int)newValueAsVector3.z, (int)min, (int)max);

                                value = newValueAsArray;
                            }
                            else if (value is Vector4 newValueAsVector4)
                            {
                                newValueAsArray[0] = Mathf.Clamp((int)newValueAsVector4.x, (int)min, (int)max);
                                newValueAsArray[1] = Mathf.Clamp((int)newValueAsVector4.y, (int)min, (int)max);
                                newValueAsArray[2] = Mathf.Clamp((int)newValueAsVector4.z, (int)min, (int)max);
                                newValueAsArray[3] = Mathf.Clamp((int)newValueAsVector4.w, (int)min, (int)max);

                                value = newValueAsArray;
                            }
                            else
                            {
                                var array = value as int[];
                                for (int i = 0; i < array.Length; i++)
                                {
                                    array[i] = Mathf.Clamp(array[i], (int)min, (int)max);
                                }
                            }
                            break;
                        }
                    case PropertyInfoDataType.FloatArray:
                        {
                            var newValueAsArray = new float[arrayLength];

                            if (value is Vector2 newValueAsVector2)
                            {
                                newValueAsArray[0] = Mathf.Clamp(newValueAsVector2.x, min, max);
                                newValueAsArray[1] = Mathf.Clamp(newValueAsVector2.y, min, max);

                                value = newValueAsArray;
                            }
                            else if (value is Vector3 newValueAsVector3)
                            {
                                newValueAsArray[0] = Mathf.Clamp(newValueAsVector3.x, min, max);
                                newValueAsArray[1] = Mathf.Clamp(newValueAsVector3.y, min, max);
                                newValueAsArray[2] = Mathf.Clamp(newValueAsVector3.z, min, max);

                                value = newValueAsArray;
                            }
                            else if (value is Vector4 newValueAsVector4)
                            {
                                newValueAsArray[0] = Mathf.Clamp(newValueAsVector4.x, min, max);
                                newValueAsArray[1] = Mathf.Clamp(newValueAsVector4.y, min, max);
                                newValueAsArray[2] = Mathf.Clamp(newValueAsVector4.z, min, max);
                                newValueAsArray[3] = Mathf.Clamp(newValueAsVector4.w, min, max);

                                value = newValueAsArray;
                            }
                            else
                            {
                                var array = value as float[];
                                for (int i = 0; i < array.Length; i++)
                                {
                                    array[i] = Mathf.Clamp(array[i], min, max);
                                }
                            }
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
        public enum InstanceHandlingType
        {
            InstanceRenderer,
            Copies,
            Prefabs
        }

        [SerializeField]
        private InstanceHandlingType instanceHandling = InstanceHandlingType.InstanceRenderer;

        public List<PropertyInfoDataWrapper> propertyInfos
        {
            get
            {
                var propertyComponent = Misc.GetOrAddComponent<MeshSyncServerProperties>(gameObject);

                return propertyComponent.propertyInfos;
            }
        }

        public virtual InstanceHandlingType InstanceHandling
        {
            get => instanceHandling;
            set => instanceHandling = value;
        }

#if AT_USE_PROBUILDER
        [SerializeField]
        private bool useProBuilder;

        public virtual bool UseProBuilder
        {
            get => useProBuilder;
            set => useProBuilder = value;
        }
#endif

        void UpdateProperties(SceneData scene)
        {
            // handle properties
            Try(() =>
            {
                var numProperties = scene.numPropertyInfos;
                if (numProperties > 0)
                {
                    lock (PropertyInfoDataWrapper.PropertyUpdateLock)
                    {
                        List<PropertyInfoDataWrapper> pendingProps = null;

                        // If there are dirty properties, make sure we set any updated values on the new ones:
                        foreach (var prop in propertyInfos)
                        {
                            if (prop.IsDirty)
                            {
                                if (pendingProps == null)
                                {
                                    pendingProps = new List<PropertyInfoDataWrapper>();
                                }
                                pendingProps.Add(prop);
                            }
                        }

                        propertyInfos.Clear();
                        for (var i = 0; i < numProperties; ++i)
                        {
                            var data = scene.GetPropertyInfo(i);
                            propertyInfos.Add(new PropertyInfoDataWrapper(data));
                        }

                        propertyInfos.Sort(new PropertyInfoDataWrapperComparer());

                        if (pendingProps != null)
                        {
                            foreach (var pendingProp in pendingProps)
                            {
                                foreach (var prop in propertyInfos)
                                {
                                    if (prop.Matches(pendingProp))
                                    {
                                        prop.NewValue = pendingProp.NewValue;
                                    }
                                }
                            }
                        }
                    }
                }
            });
        }

        private EntityRecord UpdateCurveEntity(CurvesData data, MeshSyncPlayerConfig config)
        {
#if AT_USE_SPLINES
            TransformData dtrans = data.transform;
            EntityRecord rec = UpdateTransformEntity(dtrans, config);

            GameObject go = rec.go;
            Transform trans = go.transform;

            SplineContainer splineContainer = rec.splineContainer;

            if (splineContainer == null)
            {
                splineContainer = rec.splineContainer = Misc.GetOrAddComponent<SplineContainer>(trans.gameObject);
            }

            float3[] cos = null;
            float3[] handles_left = null;
            float3[] handles_right = null;

            var numSplines = data.numSplines;

            Spline.Changed -= SplineChanged;

            var newSplines = new List<Spline>();

            for (int index = 0; index < numSplines; index++)
            {
                var spline = new Spline();
                newSplines.Add(spline);

                spline.Closed = data.IsSplineClosed(index);

                var numPoints = data.GetNumSplinePoints(index);
                m_tmpFloat3.Resize(numPoints);

                data.ReadSplineCos(index, m_tmpFloat3);
                m_tmpFloat3.CopyTo(ref cos);

                data.ReadSplineHandlesLeft(index, m_tmpFloat3);
                m_tmpFloat3.CopyTo(ref handles_left);

                data.ReadSplineHandlesRight(index, m_tmpFloat3);
                m_tmpFloat3.CopyTo(ref handles_right);

                for (int pointIndex = 0; pointIndex < cos.Length; pointIndex++)
                {
                    var co = cos[pointIndex];

                    var knot = new BezierKnot(co, handles_left[pointIndex] - co, handles_right[pointIndex] - co, Quaternion.identity);

                    spline.Add(knot);
                }
            }

            splineContainer.Branches = newSplines;

            Spline.Changed += SplineChanged;

            return rec;
#else
            return null;
#endif
        }

#if AT_USE_SPLINES
        protected virtual void SplineChanged(Spline spline, int arg2, SplineModification arg3)
        {
        }
#endif
    }
}
