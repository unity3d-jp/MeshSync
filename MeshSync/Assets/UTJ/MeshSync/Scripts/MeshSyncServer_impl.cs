using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.MeshSync
{
    public partial class MeshSyncServer
    {
        #region internal
        public enum MessageType
        {
            Unknown,
            Get,
            Set,
            Delete,
            Fence,
            Text,
            Screenshot,
        }

        public struct GetFlags
        {
            public int flags;
            public bool getTransform { get { return (flags & (1 << 0)) != 0; } }
            public bool getPoints { get { return (flags & (1 << 1)) != 0; } }
            public bool getNormals { get { return (flags & (1 << 2)) != 0; } }
            public bool getTangents { get { return (flags & (1 << 3)) != 0; } }
            public bool getUV0 { get { return (flags & (1 << 4)) != 0; } }
            public bool getUV1 { get { return (flags & (1 << 5)) != 0; } }
            public bool getColors { get { return (flags & (1 << 6)) != 0; } }
            public bool getIndices { get { return (flags & (1 << 7)) != 0; } }
            public bool getMaterialIDs { get { return (flags & (1 << 8)) != 0; } }
            public bool getBones { get { return (flags & (1 << 9)) != 0; } }
            public bool getBlendShapes { get { return (flags & (1 << 10)) != 0; } }
        }

        public struct GetMessage
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern GetFlags msGetGetFlags(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msGetGetBakeSkin(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msGetGetBakeCloth(IntPtr _this);

            public static explicit operator GetMessage(IntPtr v)
            {
                GetMessage ret;
                ret._this = v;
                return ret;
            }

            public GetFlags flags { get { return msGetGetFlags(_this); } }
            public bool bakeSkin { get { return msGetGetBakeSkin(_this) != 0; } }
            public bool bakeCloth { get { return msGetGetBakeCloth(_this) != 0; } }
        }

        public struct SetMessage
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern SceneData msSetGetSceneData(IntPtr _this);

            public static explicit operator SetMessage(IntPtr v)
            {
                SetMessage ret;
                ret._this = v;
                return ret;
            }

            public SceneData scene
            {
                get { return msSetGetSceneData(_this); }
            }
        }


        public struct DeleteMessage
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern int msDeleteGetNumTargets(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern IntPtr msDeleteGetPath(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern int msDeleteGetID(IntPtr _this, int i);

            public static explicit operator DeleteMessage(IntPtr v)
            {
                DeleteMessage ret;
                ret._this = v;
                return ret;
            }

            public int numTargets { get { return msDeleteGetNumTargets(_this); } }
            public string GetPath(int i) { return S(msDeleteGetPath(_this, i)); }
            public int GetID(int i) { return msDeleteGetID(_this, i); }
        }
        

        public struct FenceMessage
        {
            public enum FenceType
            {
                Unknown,
                SceneBegin,
                SceneEnd,
            }

            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern FenceType msFenceGetType(IntPtr _this);

            public static explicit operator FenceMessage(IntPtr v)
            {
                FenceMessage ret;
                ret._this = v;
                return ret;
            }

            public FenceType type { get { return msFenceGetType(_this); } }
        }


        public struct TextMessage
        {
            public enum TextType
            {
                Normal,
                Warning,
                Error,
            }

            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern IntPtr msTextGetText(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern TextType msTextGetType(IntPtr _this);

            public static explicit operator TextMessage(IntPtr v)
            {
                TextMessage ret;
                ret._this = v;
                return ret;
            }

            public string text { get { return S(msTextGetText(_this)); } }
            public TextType textType { get { return msTextGetType(_this); } }

            public void Print()
            {
                switch (textType) {
                    case TextType.Error:
                        Debug.LogError(text);
                        break;
                    case TextType.Warning:
                        Debug.LogWarning(text);
                        break;
                    default:
                        Debug.Log(text);
                        break;
                }

            }
        }



        public struct MeshDataFlags
        {
            public int flags;
            public bool hasRefineSettings
            {
                get { return (flags & (1 << 0)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 0)); }
            }
            public bool hasIndices
            {
                get { return (flags & (1 << 1)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 1)); }
            }
            public bool hasCounts
            {
                get { return (flags & (1 << 2)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 2)); }
            }
            public bool hasPoints
            {
                get { return (flags & (1 << 3)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 3)); }
            }
            public bool hasNormals
            {
                get { return (flags & (1 << 4)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 4)); }
            }
            public bool hasTangents
            {
                get { return (flags & (1 << 5)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 5)); }
            }
            public bool hasUV0
            {
                get { return (flags & (1 << 6)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 6)); }
            }
            public bool hasUV1
            {
                get { return (flags & (1 << 7)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 7)); }
            }
            public bool hasColors
            {
                get { return (flags & (1 << 8)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 8)); }
            }
            public bool hasMaterialIDs
            {
                get { return (flags & (1 << 9)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 9)); }
            }
            public bool hasBones
            {
                get { return (flags & (1 << 10)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 10)); }
            }
            public bool hasBlendshapes
            {
                get { return (flags & (1 << 11)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 11)); }
            }
            public bool applyTRS
            {
                get { return (flags & (1 << 12)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 12)); }
            }
        };

        public struct MaterialData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern MaterialData msMaterialCreate();
            [DllImport("MeshSyncServer")] static extern int msMaterialGetID(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMaterialSetID(IntPtr _this, int v);
            [DllImport("MeshSyncServer")] static extern IntPtr msMaterialGetName(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMaterialSetName(IntPtr _this, string v);
            [DllImport("MeshSyncServer")] static extern Color msMaterialGetColor(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMaterialSetColor(IntPtr _this, ref Color v);

            public static MaterialData Create() { return msMaterialCreate(); }

            public int id {
                get { return msMaterialGetID(_this); }
                set { msMaterialSetID(_this, value); }
            }
            public string name {
                get { return S(msMaterialGetName(_this)); }
                set { msMaterialSetName(_this, value); }
            }
            public Color color {
                get { return msMaterialGetColor(_this); }
                set { msMaterialSetColor(_this, ref value); }
            }
        }

        public struct TransformAnimationData
        {
            internal IntPtr _this;

            [DllImport("MeshSyncServer")] static extern int msTransformAGetNumTranslationSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msTransformAGetTranslationTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern Vector3 msTransformAGetTranslationValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msTransformAGetNumRotationSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msTransformAGetRotationTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern Quaternion msTransformAGetRotationValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msTransformAGetNumScaleSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msTransformAGetScaleTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern Vector3 msTransformAGetScaleValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msTransformAGetNumVisibleSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msTransformAGetVisibleTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern byte msTransformAGetVisibleValue(IntPtr _this, int i);

            public static explicit operator TransformAnimationData(IntPtr v)
            {
                TransformAnimationData ret;
                ret._this = v;
                return ret;
            }
            public static implicit operator bool(TransformAnimationData v)
            {
                return v._this != IntPtr.Zero;
            }

            public float[] translateTimes
            {
                get
                {
                    var ret = new float[msTransformAGetNumTranslationSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msTransformAGetTranslationTime(_this, i); }
                    return ret;
                }
            }
            public Vector3[] translateValues
            {
                get
                {
                    var ret = new Vector3[msTransformAGetNumTranslationSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msTransformAGetTranslationValue(_this, i); }
                    return ret;
                }
            }

            public float[] rotationTimes
            {
                get
                {
                    var ret = new float[msTransformAGetNumRotationSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msTransformAGetRotationTime(_this, i); }
                    return ret;
                }
            }
            public Quaternion[] rotationValues
            {
                get
                {
                    var ret = new Quaternion[msTransformAGetNumRotationSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msTransformAGetRotationValue(_this, i); }
                    return ret;
                }
            }

            public float[] scaleTimes
            {
                get
                {
                    var ret = new float[msTransformAGetNumScaleSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msTransformAGetScaleTime(_this, i); }
                    return ret;
                }
            }
            public Vector3[] scaleValues
            {
                get
                {
                    var ret = new Vector3[msTransformAGetNumScaleSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msTransformAGetScaleValue(_this, i); }
                    return ret;
                }
            }

            public float[] visibleTimes
            {
                get
                {
                    var ret = new float[msTransformAGetNumVisibleSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msTransformAGetVisibleTime(_this, i); }
                    return ret;
                }
            }
            public bool[] visibleValues
            {
                get
                {
                    var ret = new bool[msTransformAGetNumVisibleSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msTransformAGetVisibleValue(_this, i) != 0; }
                    return ret;
                }
            }

            public void ExportToClip(AnimationClip clip, string path, bool reduce = false)
            {
                var t = AnimationData.ToAnimatinCurve(translateTimes, translateValues, reduce);
                var r = AnimationData.ToAnimatinCurve(rotationTimes, rotationValues, reduce);
                var s = AnimationData.ToAnimatinCurve(scaleTimes, scaleValues, reduce);
                var v = AnimationData.ToAnimatinCurve(visibleTimes, visibleValues, reduce);

                var ttrans = typeof(Transform);
                var tgo = typeof(GameObject);
                clip.SetCurve(path, ttrans, "m_LocalPosition", null);
                clip.SetCurve(path, ttrans, "m_LocalRotation", null);
                clip.SetCurve(path, ttrans, "m_LocalScale", null);
                clip.SetCurve(path, ttrans, "m_IsActive", null);
                if (t[0].length > 0) clip.SetCurve(path, ttrans, "m_LocalPosition.x", t[0]);
                if (t[1].length > 0) clip.SetCurve(path, ttrans, "m_LocalPosition.y", t[1]);
                if (t[2].length > 0) clip.SetCurve(path, ttrans, "m_LocalPosition.z", t[2]);
                if (r[0].length > 0) clip.SetCurve(path, ttrans, "m_LocalRotation.x", r[0]);
                if (r[1].length > 0) clip.SetCurve(path, ttrans, "m_LocalRotation.y", r[1]);
                if (r[2].length > 0) clip.SetCurve(path, ttrans, "m_LocalRotation.z", r[2]);
                if (r[3].length > 0) clip.SetCurve(path, ttrans, "m_LocalRotation.w", r[3]);
                if (s[0].length > 0) clip.SetCurve(path, ttrans, "m_LocalScale.x", s[0]);
                if (s[1].length > 0) clip.SetCurve(path, ttrans, "m_LocalScale.y", s[1]);
                if (s[2].length > 0) clip.SetCurve(path, ttrans, "m_LocalScale.z", s[2]);
                if (v.length > 0)    clip.SetCurve(path, tgo, "m_IsActive", v);
            }
        }

        public struct CameraAnimationData
        {
            internal IntPtr _this;

            [DllImport("MeshSyncServer")] static extern int msCameraAGetNumFovSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetFovTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetFovValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msCameraAGetNumNearSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetNearTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetNearValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msCameraAGetNumFarSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetFarTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetFarValue(IntPtr _this, int i);
            
            [DllImport("MeshSyncServer")] static extern int msCameraAGetNumHApertureSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetHApertureTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetHApertureValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msCameraAGetNumVApertureSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetVApertureTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetVApertureValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msCameraAGetNumFocalLengthSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetFocalLengthTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetFocalLengthValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msCameraAGetNumFocusDistanceSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetFocusDistanceTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msCameraAGetFocusDistanceValue(IntPtr _this, int i);

            public static explicit operator CameraAnimationData(IntPtr v)
            {
                CameraAnimationData ret;
                ret._this = v;
                return ret;
            }
            public static implicit operator bool(CameraAnimationData v)
            {
                return v._this != IntPtr.Zero;
            }

            public float[] fovTimes
            {
                get
                {
                    var ret = new float[msCameraAGetNumFovSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetFovTime(_this, i); }
                    return ret;
                }
            }
            public float[] fovValues
            {
                get
                {
                    var ret = new float[msCameraAGetNumFovSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetFovValue(_this, i); }
                    return ret;
                }
            }

            public float[] nearPlaneTimes
            {
                get
                {
                    var ret = new float[msCameraAGetNumNearSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetNearTime(_this, i); }
                    return ret;
                }
            }
            public float[] nearPlaneValues
            {
                get
                {
                    var ret = new float[msCameraAGetNumNearSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetNearValue(_this, i); }
                    return ret;
                }
            }

            public float[] farPlaneTimes
            {
                get
                {
                    var ret = new float[msCameraAGetNumFarSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetFarTime(_this, i); }
                    return ret;
                }
            }
            public float[] farPlaneValues
            {
                get
                {
                    var ret = new float[msCameraAGetNumFarSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetFarValue(_this, i); }
                    return ret;
                }
            }

            public float[] horizontalApertureTimes
            {
                get
                {
                    var ret = new float[msCameraAGetNumHApertureSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetHApertureTime(_this, i); }
                    return ret;
                }
            }
            public float[] horizontalApertureValues
            {
                get
                {
                    var ret = new float[msCameraAGetNumHApertureSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetHApertureValue(_this, i); }
                    return ret;
                }
            }

            public float[] verticalApertureTimes
            {
                get
                {
                    var ret = new float[msCameraAGetNumVApertureSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetVApertureTime(_this, i); }
                    return ret;
                }
            }
            public float[] verticalApertureValues
            {
                get
                {
                    var ret = new float[msCameraAGetNumVApertureSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetVApertureValue(_this, i); }
                    return ret;
                }
            }

            public float[] focalLengthTimes
            {
                get
                {
                    var ret = new float[msCameraAGetNumFocalLengthSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetFocalLengthTime(_this, i); }
                    return ret;
                }
            }
            public float[] focalLengthValues
            {
                get
                {
                    var ret = new float[msCameraAGetNumVApertureSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetFocalLengthValue(_this, i); }
                    return ret;
                }
            }

            public float[] focusDistanceTimes
            {
                get
                {
                    var ret = new float[msCameraAGetNumFocusDistanceSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetFocusDistanceTime(_this, i); }
                    return ret;
                }
            }
            public float[] focusDistanceValues
            {
                get
                {
                    var ret = new float[msCameraAGetNumVApertureSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msCameraAGetFocusDistanceValue(_this, i); }
                    return ret;
                }
            }
            public void ExportToClip(AnimationClip clip, string path, bool reduce = false)
            {
                var fov  = AnimationData.ToAnimatinCurve(fovTimes, fovValues, reduce);
                var near = AnimationData.ToAnimatinCurve(nearPlaneTimes, nearPlaneValues, reduce);
                var far  = AnimationData.ToAnimatinCurve(farPlaneTimes, farPlaneValues, reduce);

                var tcam = typeof(Camera);
                clip.SetCurve(path, tcam, "field of view", null);
                clip.SetCurve(path, tcam, "near clip plane", null);
                clip.SetCurve(path, tcam, "far clip plane", null);
                if (fov.length > 0)  clip.SetCurve(path, tcam, "field of view", fov);
                if (near.length > 0) clip.SetCurve(path, tcam, "near clip plane", near);
                if (far.length > 0)  clip.SetCurve(path, tcam, "far clip plane", far);
            }
        }

        public struct LightAnimationData
        {
            internal IntPtr _this;

            [DllImport("MeshSyncServer")] static extern int msLightAGetNumColorSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msLightAGetColorTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern Color msLightAGetColorValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msLightAGetNumIntensitySamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msLightAGetIntensityTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msLightAGetIntensityValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msLightAGetNumRangeSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msLightAGetRangeTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msLightAGetRangeValue(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msLightAGetNumSpotAngleSamples(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msLightAGetSpotAngleTime(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern float msLightAGetSpotAngleValue(IntPtr _this, int i);


            public static explicit operator LightAnimationData(IntPtr v)
            {
                LightAnimationData ret;
                ret._this = v;
                return ret;
            }
            public static implicit operator bool(LightAnimationData v)
            {
                return v._this != IntPtr.Zero;
            }


            public float[] colorTimes
            {
                get
                {
                    var ret = new float[msLightAGetNumColorSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msLightAGetColorTime(_this, i); }
                    return ret;
                }
            }
            public Color[] colorValues
            {
                get
                {
                    var ret = new Color[msLightAGetNumColorSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msLightAGetColorValue(_this, i); }
                    return ret;
                }
            }

            public float[] intensityTimes
            {
                get
                {
                    var ret = new float[msLightAGetNumIntensitySamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msLightAGetIntensityTime(_this, i); }
                    return ret;
                }
            }
            public float[] intensityValues
            {
                get
                {
                    var ret = new float[msLightAGetNumIntensitySamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msLightAGetIntensityValue(_this, i); }
                    return ret;
                }
            }

            public float[] rangeTimes
            {
                get
                {
                    var ret = new float[msLightAGetNumRangeSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msLightAGetRangeTime(_this, i); }
                    return ret;
                }
            }
            public float[] rangeValues
            {
                get
                {
                    var ret = new float[msLightAGetNumRangeSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msLightAGetRangeValue(_this, i); }
                    return ret;
                }
            }

            public float[] spotAngleTimes
            {
                get
                {
                    var ret = new float[msLightAGetNumSpotAngleSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msLightAGetSpotAngleTime(_this, i); }
                    return ret;
                }
            }
            public float[] spotAngleValues
            {
                get
                {
                    var ret = new float[msLightAGetNumSpotAngleSamples(_this)];
                    for (int i = 0; i < ret.Length; ++i) { ret[i] = msLightAGetSpotAngleValue(_this, i); }
                    return ret;
                }
            }

            public void ExportToClip(AnimationClip clip, string path, bool reduce = false)
            {
                var color     = AnimationData.ToAnimatinCurve(colorTimes, colorValues, reduce);
                var intensity = AnimationData.ToAnimatinCurve(intensityTimes, intensityValues, reduce);
                var range     = AnimationData.ToAnimatinCurve(rangeTimes, rangeValues, reduce);
                var spot      = AnimationData.ToAnimatinCurve(spotAngleTimes, spotAngleValues, reduce);

                var tlight = typeof(Light);
                clip.SetCurve(path, tlight, "m_Color", null);
                clip.SetCurve(path, tlight, "m_Intensity", null);
                clip.SetCurve(path, tlight, "m_Range", null);
                clip.SetCurve(path, tlight, "m_SpotAngle", null);
                if (color[0].length > 0)  clip.SetCurve(path, tlight, "m_Color.r", color[0]);
                if (color[1].length > 0)  clip.SetCurve(path, tlight, "m_Color.g", color[1]);
                if (color[2].length > 0)  clip.SetCurve(path, tlight, "m_Color.b", color[2]);
                if (color[3].length > 0)  clip.SetCurve(path, tlight, "m_Color.a", color[3]);
                if (intensity.length > 0) clip.SetCurve(path, tlight, "m_Intensity", intensity);
                if (range.length > 0)     clip.SetCurve(path, tlight, "m_Range", range);
                if (spot.length > 0)      clip.SetCurve(path, tlight, "m_SpotAngle", spot);
            }
        }



        public struct AnimationData
        {
            internal IntPtr _this;

            [DllImport("MeshSyncServer")] static extern TransformAnimationData msAnimationAsTransform(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern CameraAnimationData msAnimationAsCamera(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern LightAnimationData msAnimationAsLight(IntPtr _this);


            public static explicit operator AnimationData(IntPtr v)
            {
                AnimationData ret;
                ret._this = v;
                return ret;
            }
            public static implicit operator bool(AnimationData v)
            {
                return v._this != IntPtr.Zero;
            }

            public TransformAnimationData transform
            {
                get { return msAnimationAsTransform(_this); }
            }

            public CameraAnimationData camera
            {
                get { return msAnimationAsCamera(_this); }
            }

            public LightAnimationData light
            {
                get { return msAnimationAsLight(_this); }
            }

            public void ExportToClip(AnimationClip clip, string path, bool reduce = false)
            {
                {
                    var tmp = transform;
                    if (tmp) { tmp.ExportToClip(clip, path, reduce); }
                }
                {
                    var tmp = camera;
                    if (tmp) { tmp.ExportToClip(clip, path, reduce); }
                }
                {
                    var tmp = light;
                    if (tmp) { tmp.ExportToClip(clip, path, reduce); }
                }
            }



            public static AnimationCurve[] ToAnimatinCurve(float[] times, Vector3[] values, bool reduce)
            {
                var ret = new AnimationCurve[3] {
                    new AnimationCurve(),
                    new AnimationCurve(),
                    new AnimationCurve(),
                };
                if(times.Length == 0) { return ret; }

                for (int i = 0; i < times.Length; ++i)
                {
                    var t = times[i];
                    var v = values[i];
                    ret[0].AddKey(new Keyframe(t, v.x));
                    ret[1].AddKey(new Keyframe(t, v.y));
                    ret[2].AddKey(new Keyframe(t, v.z));
#if UNITY_EDITOR
                    AnimationUtility.SetKeyLeftTangentMode(ret[0], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[0], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyLeftTangentMode(ret[1], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[1], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyLeftTangentMode(ret[2], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[2], i, AnimationUtility.TangentMode.Auto);
#endif
                }
                if(reduce)
                {
                    AnimationCurveKeyReducer.DoReduction(ret[0]);
                    AnimationCurveKeyReducer.DoReduction(ret[1]);
                    AnimationCurveKeyReducer.DoReduction(ret[2]);
                }
                return ret;
            }

            public static AnimationCurve[] ToAnimatinCurve(float[] times, Quaternion[] values, bool reduce)
            {
                var ret = new AnimationCurve[4] {
                    new AnimationCurve(),
                    new AnimationCurve(),
                    new AnimationCurve(),
                    new AnimationCurve(),
                };
                if (times.Length == 0) { return ret; }

                for (int i = 0; i < times.Length; ++i)
                {
                    var t = times[i];
                    var v = values[i];
                    ret[0].AddKey(new Keyframe(t, v.x));
                    ret[1].AddKey(new Keyframe(t, v.y));
                    ret[2].AddKey(new Keyframe(t, v.z));
                    ret[3].AddKey(new Keyframe(t, v.w));
#if UNITY_EDITOR
                    AnimationUtility.SetKeyLeftTangentMode(ret[0], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[0], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyLeftTangentMode(ret[1], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[1], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyLeftTangentMode(ret[2], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[2], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyLeftTangentMode(ret[3], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[3], i, AnimationUtility.TangentMode.Auto);
#endif
                }
                if (reduce)
                {
                    AnimationCurveKeyReducer.DoReduction(ret[0]);
                    AnimationCurveKeyReducer.DoReduction(ret[1]);
                    AnimationCurveKeyReducer.DoReduction(ret[2]);
                    AnimationCurveKeyReducer.DoReduction(ret[3]);
                }
                return ret;
            }

            public static AnimationCurve[] ToAnimatinCurve(float[] times, Color[] values, bool reduce)
            {
                var ret = new AnimationCurve[4] {
                    new AnimationCurve(),
                    new AnimationCurve(),
                    new AnimationCurve(),
                    new AnimationCurve(),
                };
                if (times.Length == 0) { return ret; }

                for (int i = 0; i < times.Length; ++i)
                {
                    var t = times[i];
                    var v = values[i];
                    ret[0].AddKey(new Keyframe(t, v.r));
                    ret[1].AddKey(new Keyframe(t, v.g));
                    ret[2].AddKey(new Keyframe(t, v.b));
                    ret[3].AddKey(new Keyframe(t, v.a));
#if UNITY_EDITOR
                    AnimationUtility.SetKeyLeftTangentMode(ret[0], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[0], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyLeftTangentMode(ret[1], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[1], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyLeftTangentMode(ret[2], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[2], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyLeftTangentMode(ret[3], i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret[3], i, AnimationUtility.TangentMode.Auto);
#endif
                }
                if (reduce)
                {
                    AnimationCurveKeyReducer.DoReduction(ret[0]);
                    AnimationCurveKeyReducer.DoReduction(ret[1]);
                    AnimationCurveKeyReducer.DoReduction(ret[2]);
                    AnimationCurveKeyReducer.DoReduction(ret[3]);
                }
                return ret;
            }

            public static AnimationCurve ToAnimatinCurve(float[] times, float[] values, bool reduce)
            {
                var ret = new AnimationCurve();
                if (times.Length == 0) { return ret; }

                for (int i = 0; i < times.Length; ++i)
                {
                    var t = times[i];
                    var v = values[i];
                    ret.AddKey(new Keyframe(t, v));
#if UNITY_EDITOR
                    AnimationUtility.SetKeyLeftTangentMode(ret, i, AnimationUtility.TangentMode.Auto);
                    AnimationUtility.SetKeyRightTangentMode(ret, i, AnimationUtility.TangentMode.Auto);
#endif
                }
                if (reduce)
                {
                    AnimationCurveKeyReducer.DoReduction(ret);
                }
                return ret;
            }

            public static AnimationCurve ToAnimatinCurve(float[] times, bool[] values, bool reduce)
            {
                var ret = new AnimationCurve();
                if (times.Length == 0) { return ret; }

                for (int i = 0; i < times.Length; ++i)
                {
                    var t = times[i];
                    var v = values[i];
                    ret.AddKey(new Keyframe(t, v ? 0.0f : 1.0f));
#if UNITY_EDITOR
                    AnimationUtility.SetKeyLeftTangentMode(ret, i, AnimationUtility.TangentMode.Constant);
                    AnimationUtility.SetKeyRightTangentMode(ret, i, AnimationUtility.TangentMode.Constant);
#endif
                }
                if (reduce)
                {
                    AnimationCurveKeyReducer.DoReduction(ret);
                }
                return ret;
            }
        }


        public struct TransformData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern TransformData msTransformCreate();
            [DllImport("MeshSyncServer")] static extern int msTransformGetID(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msTransformSetID(IntPtr _this, int v);
            [DllImport("MeshSyncServer")] static extern int msTransformGetIndex(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msTransformSetIndex(IntPtr _this, int v);
            [DllImport("MeshSyncServer")] static extern IntPtr msTransformGetPath(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msTransformSetPath(IntPtr _this, string v);
            [DllImport("MeshSyncServer")] static extern Vector3 msTransformGetPosition(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msTransformSetPosition(IntPtr _this, Vector3 v);
            [DllImport("MeshSyncServer")] static extern Quaternion msTransformGetRotation(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msTransformSetRotation(IntPtr _this, Quaternion v);
            [DllImport("MeshSyncServer")] static extern Vector3 msTransformGetScale(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msTransformSetScale(IntPtr _this, Vector3 v);
            [DllImport("MeshSyncServer")] static extern byte msTransformGetVisible(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msTransformSetVisible(IntPtr _this, byte v);
            [DllImport("MeshSyncServer")] static extern IntPtr msTransformGetReference(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msTransformSetReference(IntPtr _this, string v);
            [DllImport("MeshSyncServer")] static extern AnimationData msTransformGetAnimation(IntPtr _this);

            public static explicit operator TransformData(IntPtr v)
            {
                TransformData ret;
                ret._this = v;
                return ret;
            }

            public static TransformData Create()
            {
                return msTransformCreate();
            }

            public int id
            {
                get { return msTransformGetID(_this); }
                set { msTransformSetID(_this, value); }
            }
            public int index
            {
                get { return msTransformGetIndex(_this); }
                set { msTransformSetIndex(_this, value); }
            }
            public string path
            {
                get { return S(msTransformGetPath(_this)); }
                set { msTransformSetPath(_this, value); }
            }
            public Vector3 position
            {
                get { return msTransformGetPosition(_this); }
                set { msTransformSetPosition(_this, value); }
            }
            public Quaternion rotation
            {
                get { return msTransformGetRotation(_this); }
                set { msTransformSetRotation(_this, value); }
            }
            public Vector3 scale
            {
                get { return msTransformGetScale(_this); }
                set { msTransformSetScale(_this, value); }
            }
            public bool visible
            {
                get { return msTransformGetVisible(_this) != 0; }
                set { msTransformSetVisible(_this, (byte)(value ? 1 : 0)); }
            }
            public string reference
            {
                get { return S(msTransformGetReference(_this)); }
                set { msTransformSetReference(_this, value); }
            }
            public AnimationData animation
            {
                get { return msTransformGetAnimation(_this); }
            }
        }

        public struct CameraData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern CameraData msCameraCreate();
            [DllImport("MeshSyncServer")] static extern byte msCameraIsOrtho(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msCameraSetOrtho(IntPtr _this, byte v);
            [DllImport("MeshSyncServer")] static extern float msCameraGetFov(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msCameraSetFov(IntPtr _this, float v);
            [DllImport("MeshSyncServer")] static extern float msCameraGetNearPlane(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msCameraSetNearPlane(IntPtr _this, float v);
            [DllImport("MeshSyncServer")] static extern float msCameraGetFarPlane(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msCameraSetFarPlane(IntPtr _this, float v);
            [DllImport("MeshSyncServer")] static extern float msCameraGetHorizontalAperture(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msCameraSetHorizontalAperture(IntPtr _this, float v);
            [DllImport("MeshSyncServer")] static extern float msCameraGetVerticalAperture(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msCameraSetVerticalAperture(IntPtr _this, float v);
            [DllImport("MeshSyncServer")] static extern float msCameraGetFocalLength(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msCameraSetFocalLength(IntPtr _this, float v);
            [DllImport("MeshSyncServer")] static extern float msCameraGetFocusDistance(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msCameraSetFocusDistance(IntPtr _this, float v);


            public static explicit operator CameraData(IntPtr v)
            {
                CameraData ret;
                ret._this = v;
                return ret;
            }

            public static CameraData Create()
            {
                return msCameraCreate();
            }

            public TransformData transform
            {
                get { return (TransformData)_this; }
            }

            public bool orthographic
            {
                get { return msCameraIsOrtho(_this) != 0; }
                set { msCameraSetOrtho(_this, (byte)(value ? 1 : 0)); }
            }
            public float fov
            {
                get { return msCameraGetFov(_this); }
                set { msCameraSetFov(_this, value); }
            }
            public float nearClipPlane
            {
                get { return msCameraGetNearPlane(_this); }
                set { msCameraSetNearPlane(_this, value); }
            }
            public float farClipPlane
            {
                get { return msCameraGetFarPlane(_this); }
                set { msCameraSetFarPlane(_this, value); }
            }
            public float horizontalAperture
            {
                get { return msCameraGetHorizontalAperture(_this); }
                set { msCameraSetHorizontalAperture(_this, value); }
            }
            public float verticalAperture
            {
                get { return msCameraGetVerticalAperture(_this); }
                set { msCameraSetVerticalAperture(_this, value); }
            }
            public float focalLength
            {
                get { return msCameraGetFocalLength(_this); }
                set { msCameraSetFocalLength(_this, value); }
            }
            public float focusDistance
            {
                get { return msCameraGetFocusDistance(_this); }
                set { msCameraSetFocusDistance(_this, value); }
            }
        }

        public struct LightData
        {
            internal IntPtr _this;

            [DllImport("MeshSyncServer")] static extern LightData msLightCreate();
            [DllImport("MeshSyncServer")] static extern LightType msLightGetType(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msLightSetType(IntPtr _this, LightType v);
            [DllImport("MeshSyncServer")] static extern Color msLightGetColor(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msLightSetColor(IntPtr _this, Color v);
            [DllImport("MeshSyncServer")] static extern float msLightGetIntensity(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msLightSetIntensity(IntPtr _this, float v);
            [DllImport("MeshSyncServer")] static extern float msLightGetRange(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msLightSetRange(IntPtr _this, float v);
            [DllImport("MeshSyncServer")] static extern float msLightGetSpotAngle(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msLightSetSpotAngle(IntPtr _this, float v);

            public static explicit operator LightData(IntPtr v)
            {
                LightData ret;
                ret._this = v;
                return ret;
            }

            public static LightData Create()
            {
                return msLightCreate();
            }

            public TransformData transform
            {
                get { return (TransformData)_this; }
            }

            public LightType type
            {
                get { return msLightGetType(_this); }
                set { msLightSetType(_this, value); }
            }
            public Color color
            {
                get { return msLightGetColor(_this); }
                set { msLightSetColor(_this, value); }
            }
            public float intensity
            {
                get { return msLightGetIntensity(_this); }
                set { msLightSetIntensity(_this, value); }
            }
            public float range
            {
                get { return msLightGetRange(_this); }
                set { msLightSetRange(_this, value); }
            }
            public float spotAngle
            {
                get { return msLightGetSpotAngle(_this); }
                set { msLightSetSpotAngle(_this, value); }
            }
        }

        public struct MeshData
        {
            internal IntPtr _this;

            [DllImport("MeshSyncServer")] static extern MeshData msMeshCreate();
            [DllImport("MeshSyncServer")] static extern MeshDataFlags msMeshGetFlags(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMeshSetFlags(IntPtr _this, MeshDataFlags v);
            [DllImport("MeshSyncServer")] static extern int msMeshGetNumPoints(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msMeshGetNumIndices(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msMeshGetNumSplits(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMeshReadPoints(IntPtr _this, IntPtr dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msMeshWritePoints(IntPtr _this, Vector3[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadNormals(IntPtr _this, IntPtr dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteNormals(IntPtr _this, Vector3[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadTangents(IntPtr _this, IntPtr dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteTangents(IntPtr _this, Vector4[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadUV0(IntPtr _this, IntPtr dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msMeshReadUV1(IntPtr _this, IntPtr dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteUV0(IntPtr _this, Vector2[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteUV1(IntPtr _this, Vector2[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadColors(IntPtr _this, IntPtr dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteColors(IntPtr _this, Color[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadWeights4(IntPtr _this, IntPtr dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteWeights4(IntPtr _this, BoneWeight[] weights, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadIndices(IntPtr _this, IntPtr dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteIndices(IntPtr _this, int[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteSubmeshTriangles(IntPtr _this, int[] v, int size, int materialID);

            [DllImport("MeshSyncServer")] static extern int msMeshGetNumBones(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern IntPtr msMeshGetRootBonePath(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMeshSetRootBonePath(IntPtr _this, string v);
            [DllImport("MeshSyncServer")] static extern IntPtr msMeshGetBonePath(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern void msMeshSetBonePath(IntPtr _this, string v, int i);
            [DllImport("MeshSyncServer")] static extern void msMeshReadBindPoses(IntPtr _this, Matrix4x4[] v);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteBindPoses(IntPtr _this, Matrix4x4[] v, int size);

            [DllImport("MeshSyncServer")] static extern void msMeshSetLocal2World(IntPtr _this, ref Matrix4x4 v);
            [DllImport("MeshSyncServer")] static extern void msMeshSetWorld2Local(IntPtr _this, ref Matrix4x4 v);

            [DllImport("MeshSyncServer")] static extern SplitData msMeshGetSplit(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern int msMeshGetNumSubmeshes(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern SubmeshData msMeshGetSubmesh(IntPtr _this, int i);

            [DllImport("MeshSyncServer")] static extern int msMeshGetNumBlendShapes(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern BlendShapeData msMeshGetBlendShapeData(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern BlendShapeData msMeshAddBlendShape(IntPtr _this, string name);


            public static MeshData Create()
            {
                return msMeshCreate();
            }

            public static explicit operator MeshData(IntPtr v)
            {
                MeshData ret;
                ret._this = v;
                return ret;
            }

            public TransformData transform
            {
                get { return (TransformData)_this; }
            }

            public MeshDataFlags flags
            {
                get { return msMeshGetFlags(_this); }
                set { msMeshSetFlags(_this, value); }
            }

            public int numPoints { get { return msMeshGetNumPoints(_this); } }
            public int numIndices { get { return msMeshGetNumIndices(_this); } }
            public int numSplits { get { return msMeshGetNumSplits(_this); } }

            public void ReadPoints(PinnedList<Vector3> dst, SplitData split) { msMeshReadPoints(_this, dst, split); }
            public void ReadNormals(PinnedList<Vector3> dst, SplitData split) { msMeshReadNormals(_this, dst, split); }
            public void ReadTangents(PinnedList<Vector4> dst, SplitData split) { msMeshReadTangents(_this, dst, split); }
            public void ReadUV0(PinnedList<Vector2> dst, SplitData split) { msMeshReadUV0(_this, dst, split); }
            public void ReadUV1(PinnedList<Vector2> dst, SplitData split) { msMeshReadUV1(_this, dst, split); }
            public void ReadColors(PinnedList<Color> dst, SplitData split) { msMeshReadColors(_this, dst, split); }
            public void ReadBoneWeights(IntPtr dst, SplitData split) { msMeshReadWeights4(_this, dst, split); }
            public void ReadIndices(IntPtr dst, SplitData split) { msMeshReadIndices(_this, dst, split); }

            public void WritePoints(Vector3[] v) { msMeshWritePoints(_this, v, v.Length); }
            public void WriteNormals(Vector3[] v) { msMeshWriteNormals(_this, v, v.Length); }
            public void WriteTangents(Vector4[] v) { msMeshWriteTangents(_this, v, v.Length); }
            public void WriteUV0(Vector2[] v) { msMeshWriteUV0(_this, v, v.Length); }
            public void WriteUV1(Vector2[] v) { msMeshWriteUV1(_this, v, v.Length); }
            public void WriteColors(Color[] v) { msMeshWriteColors(_this, v, v.Length); }
            public void WriteWeights(BoneWeight[] v) { msMeshWriteWeights4(_this, v, v.Length); }
            public void WriteIndices(int[] v) { msMeshWriteIndices(_this, v, v.Length); }

            public Matrix4x4 local2world { set { msMeshSetLocal2World(_this, ref value); } }
            public Matrix4x4 world2local { set { msMeshSetWorld2Local(_this, ref value); } }

            public SplitData GetSplit(int i) { return msMeshGetSplit(_this, i); }
            public void WriteSubmeshTriangles(int[] indices, int materialID)
            {
                msMeshWriteSubmeshTriangles(_this, indices, indices.Length, materialID);
            }

            public int numBones
            {
                get { return msMeshGetNumBones(_this); }
            }
            public string rootBonePath
            {
                get { return S(msMeshGetRootBonePath(_this)); }
                set { msMeshSetRootBonePath(_this, value); }
            }
            public Matrix4x4[] bindposes
            {
                get
                {
                    var ret = new Matrix4x4[numBones];
                    msMeshReadBindPoses(_this, ret);
                    return ret;
                }
                set { msMeshWriteBindPoses(_this, value, value.Length); }
            }
            public void SetBonePaths(Transform[] bones)
            {
                int n = bones.Length;
                for (int i = 0; i < n; ++i)
                {
                    string path = BuildPath(bones[i]);
                    msMeshSetBonePath(_this, path, i);
                }
            }
            public string[] GetBonePaths()
            {
                int n = numBones;
                var ret = new string[n];
                for (int i = 0; i < n; ++i)
                {
                    ret[i] = S(msMeshGetBonePath(_this, i));
                }
                return ret;
            }

            public int numSubmeshes { get { return msMeshGetNumSubmeshes(_this); } }
            public SubmeshData GetSubmesh(int i)
            {
                return msMeshGetSubmesh(_this, i);
            }

            public int numBlendShapes { get { return msMeshGetNumBlendShapes(_this); } }
            public BlendShapeData GetBlendShapeData(int i)
            {
                return msMeshGetBlendShapeData(_this, i);
            }
            public BlendShapeData AddBlendShape(string name)
            {
                return msMeshAddBlendShape(_this, name);
            }
        };

        public struct SplitData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern int msSplitGetNumPoints(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msSplitGetNumIndices(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msSplitGetNumSubmeshes(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern SubmeshData msSplitGetSubmesh(IntPtr _this, int i);

            public int numPoints { get { return msSplitGetNumPoints(_this); } }
            public int numIndices { get { return msSplitGetNumIndices(_this); } }
            public int numSubmeshes { get { return msSplitGetNumSubmeshes(_this); } }

            public SubmeshData GetSubmesh(int i)
            {
                return msSplitGetSubmesh(_this, i);
            }
        }

        public struct SubmeshData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern int msSubmeshGetNumIndices(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msSubmeshGetMaterialID(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msSubmeshReadIndices(IntPtr _this, int[] dst);

            public int numIndices { get { return msSubmeshGetNumIndices(_this); } }
            public int materialID { get { return msSubmeshGetMaterialID(_this); } }
            public int[] indices
            {
                get
                {
                    var ret = new int[numIndices];
                    msSubmeshReadIndices(_this, ret);
                    return ret;
                }
            }
        }

        public struct BlendShapeData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern IntPtr msBlendShapeGetName(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msBlendShapeGetWeight(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msBlendShapeGetNumFrames(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern float msBlendShapeGetFrameWeight(IntPtr _this, int f);
            [DllImport("MeshSyncServer")] static extern void msBlendShapeReadPoints(IntPtr _this, int f, Vector3[] dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msBlendShapeReadNormals(IntPtr _this, int f, Vector3[] dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msBlendShapeReadTangents(IntPtr _this, int f, Vector3[] dst, SplitData split);
            [DllImport("MeshSyncServer")] static extern void msBlendShapeAddFrame(IntPtr _this, float weight, int num, Vector3[] v, Vector3[] n, Vector3[] t);

            public string name
            {
                get { return S(msBlendShapeGetName(_this)); }
            }
            public float weight
            {
                get { return msBlendShapeGetWeight(_this); }
            }
            public float numFrames
            {
                get { return msBlendShapeGetNumFrames(_this); }
            }
            public float GetWeight(int f) { return msBlendShapeGetFrameWeight(_this, f); }
            public void ReadPoints(int f, Vector3[] dst, SplitData split) { msBlendShapeReadPoints(_this, f, dst, split); }
            public void ReadNormals(int f, Vector3[] dst, SplitData split) { msBlendShapeReadNormals(_this, f, dst, split); }
            public void ReadTangents(int f, Vector3[] dst, SplitData split) { msBlendShapeReadTangents(_this, f, dst, split); }

            public void AddFrame(float w, Vector3[] v, Vector3[] n, Vector3[] t)
            {
                msBlendShapeAddFrame(_this, w, v.Length, v, n, t);
            }
        }


        public struct SceneData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern IntPtr msSceneGetName(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msSceneGetNumTransforms(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern TransformData msSceneGetTransformData(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern int msSceneGetNumCameras(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern CameraData msSceneGetCameraData(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern int msSceneGetNumLights(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern LightData msSceneGetLightData(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern int msSceneGetNumMeshes(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern MeshData msSceneGetMeshData(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern int msSceneGetNumMaterials(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern MaterialData msSceneGetMaterialData(IntPtr _this, int i);

            public string name { get { return S(msSceneGetName(_this)); } }
            public int numTransforms { get { return msSceneGetNumTransforms(_this); } }
            public int numCameras { get { return msSceneGetNumCameras(_this); } }
            public int numLights { get { return msSceneGetNumLights(_this); } }
            public int numMeshes { get { return msSceneGetNumMeshes(_this); } }
            public int numMaterials { get { return msSceneGetNumMaterials(_this); } }

            public TransformData GetTransform(int i) { return msSceneGetTransformData(_this, i); }
            public CameraData GetCamera(int i) { return msSceneGetCameraData(_this, i); }
            public LightData GetLight(int i) { return msSceneGetLightData(_this, i); }
            public MeshData GetMesh(int i) { return msSceneGetMeshData(_this, i); }
            public MaterialData GetMaterial(int i) { return msSceneGetMaterialData(_this, i); }
        }



        public struct ServerSettings
        {
            public int max_queue;
            public int max_threads;
            public ushort port;
            public uint mesh_split_unit;

            public static ServerSettings default_value
            {
                get
                {
                    return new ServerSettings
                    {
                        max_queue = 256,
                        max_threads = 8,
                        port = 8080,
#if UNITY_2017_3_OR_NEWER
                        mesh_split_unit = 0xffffffff,
#else
                        mesh_split_unit = 65000,
#endif
                    };
                }
            }
        }

        [DllImport("MeshSyncServer")] static extern IntPtr msServerStart(ref ServerSettings settings);
        [DllImport("MeshSyncServer")] static extern void msServerStop(IntPtr sv);

        delegate void msMessageHandler(MessageType type, IntPtr data);
        [DllImport("MeshSyncServer")] static extern int msServerGetNumMessages(IntPtr sv);
        [DllImport("MeshSyncServer")] static extern int msServerProcessMessages(IntPtr sv, msMessageHandler handler);

        [DllImport("MeshSyncServer")] static extern void msServerBeginServe(IntPtr sv);
        [DllImport("MeshSyncServer")] static extern void msServerEndServe(IntPtr sv);
        [DllImport("MeshSyncServer")] static extern void msServerServeTransform(IntPtr sv, TransformData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeCamera(IntPtr sv, CameraData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeLight(IntPtr sv, LightData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeMesh(IntPtr sv, MeshData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeMaterial(IntPtr sv, MaterialData data);
        [DllImport("MeshSyncServer")] static extern void msServerSetScreenshotFilePath(IntPtr sv, string path);

        static void SwitchBits(ref int flags, bool f, int bit)
        {

            if (f) { flags |= bit; }
            else { flags &= ~bit; }
        }

        public static IntPtr RawPtr(Array v)
        {
            return v == null ? IntPtr.Zero : Marshal.UnsafeAddrOfPinnedArrayElement(v, 0);
        }
        public static string S(IntPtr cstring)
        {
            return cstring == IntPtr.Zero ? "" : Marshal.PtrToStringAnsi(cstring);
        }

        [Serializable]
        public class Record
        {
            public int index;
            public GameObject go;
            public Mesh origMesh;
            public Mesh editMesh;
            public int[] materialIDs = new int[0];
            public int[] submeshCounts = new int[0];
            public string reference;
            public bool recved = false;

            // return true if modified
            public bool BuildMaterialData(MeshData md)
            {
                int num_submeshes = md.numSubmeshes;
                if(num_submeshes == 0) { return false; }

                var mids = new int[num_submeshes];
                for (int i = 0; i < num_submeshes; ++i)
                {
                    mids[i] = md.GetSubmesh(i).materialID;
                }

                int num_splits = md.numSplits;
                var scs = new int[num_splits];
                for (int i = 0; i < num_splits; ++i)
                {
                    scs[i] = md.GetSplit(i).numSubmeshes;
                }

                bool ret = !materialIDs.SequenceEqual(mids) || !submeshCounts.SequenceEqual(scs);
                materialIDs = mids;
                submeshCounts = scs;
                return ret;
            }

            public int maxMaterialID
            {
                get
                {
                    return materialIDs.Length > 0 ? materialIDs.Max() : 0;
                }
            }
        }

        [Serializable]
        public class MaterialHolder
        {
            public int id;
            public string name;
            public Color color = Color.white;
            public Material material;
        }



        // thanks: http://techblog.sega.jp/entry/2016/11/28/100000
        public class AnimationCurveKeyReducer
        {
            static public void DoReduction(AnimationCurve in_curve, float eps = 0.001f)
            {
                if (in_curve.keys.Length <= 2) return;

                var del_indexes = GetDeleteKeyIndex(in_curve.keys, eps).ToArray();
                foreach (var del_idx in del_indexes.Reverse()) in_curve.RemoveKey(del_idx);
            }

            static IEnumerable<int> GetDeleteKeyIndex(Keyframe[] keys, float eps)
            {
                for (int s_idx = 0, i = 1; i < keys.Length - 1; i++)
                {
                    if (IsInterpolationValue(keys[s_idx], keys[i + 1], keys[i], eps))
                    {
                        yield return i;
                    }
                    else
                    {
                        s_idx = i;
                    }
                }
            }

            static bool IsInterpolationValue(Keyframe key1, Keyframe key2, Keyframe comp, float eps)
            {
                var val1 = GetValueFromTime(key1, key2, comp.time);

                if (eps < System.Math.Abs(comp.value - val1)) return false;

                var time = key1.time + (comp.time - key1.time) * 0.5f;
                val1 = GetValueFromTime(key1, comp, time);
                var val2 = GetValueFromTime(key1, key2, time);

                return (System.Math.Abs(val2 - val1) <= eps) ? true : false;
            }

            static float GetValueFromTime(Keyframe key1, Keyframe key2, float time)
            {
                float t;
                float a, b, c;
                float kd, vd;

                if (key1.outTangent == Mathf.Infinity) return key1.value;

                kd = key2.time - key1.time;
                vd = key2.value - key1.value;
                t = (time - key1.time) / kd;

                a = -2 * vd + kd * (key1.outTangent + key2.inTangent);
                b = 3 * vd - kd * (2 * key1.outTangent + key2.inTangent);
                c = kd * key1.outTangent;

                return key1.value + t * (t * (a * t + b) + c);
            }
        }
        #endregion
    }
}