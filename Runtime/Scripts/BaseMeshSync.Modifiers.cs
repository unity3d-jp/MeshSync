using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Unity.MeshSync;
using UnityEngine;

namespace Unity.MeshSync
{
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

     
            //var path = data.path;
            //switch (data.type)
            //{
            //    case PropertyInfoData.RecordType.Int:
            //        {
            //            var test = data.ValueInt;
            //            break;
            //        }
            //    case PropertyInfoData.RecordType.Float:
            //        {
            //            var test = data.ValueFloat;
            //            break;
            //        }
            //}
        }

        //internal abstract class ModifierInfo
        //{
        //    public string Name { get; private set; }

        //    public virtual void Set(VariantData data)
        //    {
        //        Name = data.name;
        //    }

        //    public enum ModifierType
        //    {
        //        Float,
        //        Int,
        //        Vector
        //    }

        //    public ModifierType Type { get; protected set; }
        //}

        //internal class FloatModifierInfo : ModifierInfo
        //{
        //    public float Value;
        //    public float Min = Mathf.NegativeInfinity;
        //    public float Max = Mathf.Infinity;
        //    public override void Set(VariantData data)
        //    {
        //        base.Set(data);
        //        Value = data.floatValue;
        //        Type = ModifierType.Float;
        //    }
        //}

        //internal class IntModifierInfo : ModifierInfo
        //{
        //    public int Value;
        //    public int Min = int.MinValue;
        //    public int Max = int.MaxValue;
        //    public override void Set(VariantData data)
        //    {
        //        base.Set(data);
        //        Value = data.intValue;
        //        Type = ModifierType.Int;
        //    }
        //}

        //internal class VectorModifierInfo : ModifierInfo
        //{
        //    public Vector3 Value;
        //    public Vector3 Min = Vector3.negativeInfinity;
        //    public Vector3 Max = Vector3.positiveInfinity;
        //    public override void Set(VariantData data)
        //    {
        //        base.Set(data);
        //        Value = data.vector3Value;
        //        Type = ModifierType.Vector;
        //    }
        //}

        //internal class ModifierInfoFactory
        //{
        //    public static ModifierInfo Create(VariantData data)
        //    {
        //        switch (data.type)
        //        {
        //            case VariantData.Type.Float:
        //                return new FloatModifierInfo();
        //            case VariantData.Type.Float3:
        //                return new VectorModifierInfo();
        //            case VariantData.Type.Int:
        //                return new IntModifierInfo();
        //            default:
        //                throw new ArgumentOutOfRangeException();
        //        }
        //    }
        //}

        //internal Dictionary<GameObject, HashSet<ModifierInfo>> modifiersInfo = new Dictionary<GameObject, HashSet<ModifierInfo>>();

        //private void HandleModifiers(TransformData data, GameObject go)
        //{
        //    //var modifiersProperty = data.FindUserProperty("modifiers");
        //    //if (modifiersProperty.self == IntPtr.Zero)
        //    //{
        //    //    return;
        //    //}

        //    //modifiersInfo.Clear();

        //    //var modifierNames = modifiersProperty.stringValue.Split('\n');

        //    //var index = 0;
        //    //foreach (var modifierName in modifierNames)
        //    //{
        //    //    if (string.IsNullOrEmpty(modifierName))
        //    //        continue;

        //    //    Debug.LogFormat("[MeshSync] Modifier{0}: {1}", index++, modifierName);

        //    //    var modifier = data.FindUserProperty(modifierName);

        //    //    if (modifier.self == IntPtr.Zero)
        //    //    {
        //    //        Debug.LogWarningFormat("[MeshSync] Modifier {0} found in manifest but does not exist in properties", modifierName);
        //    //        continue;
        //    //    }

        //    //    var modifierInfo = ModifierInfoFactory.Create(modifier);
        //    //    modifierInfo.Set(modifier);
        //    //    if (!modifiersInfo.TryGetValue(go, out HashSet<ModifierInfo> modifiers))
        //    //    {
        //    //        modifiers = new HashSet<ModifierInfo>();
        //    //        modifiersInfo.Add(go, modifiers);
        //    //    }

        //    //    modifiers.Add(modifierInfo);
        //    //}
        //}
    }
}
