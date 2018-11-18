using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.MeshSync
{
    public partial class MeshSyncServer
    {
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
                        max_queue = 512,
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

        [DllImport("MeshSyncServer")] static extern IntPtr msServerGetVersion();
        [DllImport("MeshSyncServer")] static extern IntPtr msServerStart(ref ServerSettings settings);
        [DllImport("MeshSyncServer")] static extern void msServerStop(IntPtr _this);

        delegate void msMessageHandler(MessageType type, IntPtr data);
        [DllImport("MeshSyncServer")] static extern int msServerGetNumMessages(IntPtr _this);
        [DllImport("MeshSyncServer")] static extern int msServerProcessMessages(IntPtr _this, msMessageHandler handler);

        [DllImport("MeshSyncServer")] static extern void msServerBeginServe(IntPtr _this);
        [DllImport("MeshSyncServer")] static extern void msServerEndServe(IntPtr _this);
        [DllImport("MeshSyncServer")] static extern void msServerServeTransform(IntPtr _this, TransformData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeCamera(IntPtr _this, CameraData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeLight(IntPtr _this, LightData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeMesh(IntPtr _this, MeshData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeTexture(IntPtr _this, TextureData data);
        [DllImport("MeshSyncServer")] static extern void msServerServeMaterial(IntPtr _this, MaterialData data);
        [DllImport("MeshSyncServer")] static extern void msServerSetFileRootPath(IntPtr _this, string path);
        [DllImport("MeshSyncServer")] static extern void msServerSetScreenshotFilePath(IntPtr _this, string path);
        [DllImport("MeshSyncServer")] static extern void msServerNotifyPoll(IntPtr _this, PollMessage.PollType t);

        [Serializable]
        public class EntityRecord
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
            public int index;
            public string shader;
            public Color color = Color.white;
            public Material material;
            public int materialIID;
        }

        [Serializable]
        public class TextureHolder
        {
            public int id;
            public string name;
            public Texture2D texture;
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


    }
}