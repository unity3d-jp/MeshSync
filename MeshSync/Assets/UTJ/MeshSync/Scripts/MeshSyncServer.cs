using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{
    [ExecuteInEditMode]
    public class MeshSyncServer : MonoBehaviour, ISerializationCallbackReceiver
    {
        #region internal
#if UNITY_EDITOR
        [MenuItem("GameObject/MeshSync/Create Server", false, 10)]
        public static void CreateMeshSyncServer(MenuCommand menuCommand)
        {
            var go = new GameObject();
            go.name = "MeshSyncServer";
            go.AddComponent<MeshSyncServer>();
            Undo.RegisterCreatedObjectUndo(go, "MeshSyncServer");
        }
#endif

        public enum MessageType
        {
            Unknown,
            Get,
            Set,
            Delete,
            Screenshot,
        }

        public struct GetFlags
        {
            public int flags;
            public bool getTransform { get { return (flags & (1 << 0)) != 0; } }
            public bool getPoints { get { return (flags & (1 << 1)) != 0; } }
            public bool getNormals { get { return (flags & (1 << 2)) != 0; } }
            public bool getTangents { get { return (flags & (1 << 3)) != 0; } }
            public bool getUV { get { return (flags & (1 << 4)) != 0; } }
            public bool getIndices { get { return (flags & (1 << 5)) != 0; } }
            public bool getMaterialIDs { get { return (flags & (1 << 6)) != 0; } }
            public bool getBones { get { return (flags & (1 << 7)) != 0; } }
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
            [DllImport("MeshSyncServer")] static extern int msSetGetNumMeshes(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern MeshData msSetGetMeshData(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern int msSetGetNumTransforms(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern TransformData msSetGetTransformData(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern int msSetGetNumCameras(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern CameraData msSetGetCameraData(IntPtr _this, int i);

            public static explicit operator SetMessage(IntPtr v)
            {
                SetMessage ret;
                ret._this = v;
                return ret;
            }

            public int numMeshes { get { return msSetGetNumMeshes(_this); } }
            public int numTransforms { get { return msSetGetNumTransforms(_this); } }
            public int numCameras { get { return msSetGetNumCameras(_this); } }

            public MeshData GetMesh(int i) { return msSetGetMeshData(_this, i); }
            public TransformData GetTransform(int i) { return msSetGetTransformData(_this, i); }
            public CameraData GetCamera(int i) { return msSetGetCameraData(_this, i); }
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


        public struct MeshDataFlags
        {
            public int flags;
            public bool visible
            {
                get { return (flags & (1 << 0)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 0)); }
            }
            public bool hasRefineSettings
            {
                get { return (flags & (1 << 1)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 1)); }
            }
            public bool hasIndices
            {
                get { return (flags & (1 << 2)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 2)); }
            }
            public bool hasCounts
            {
                get { return (flags & (1 << 3)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 3)); }
            }
            public bool hasPoints
            {
                get { return (flags & (1 << 4)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 4)); }
            }
            public bool hasNormals
            {
                get { return (flags & (1 << 5)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 5)); }
            }
            public bool hasTangents
            {
                get { return (flags & (1 << 6)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 6)); }
            }
            public bool hasUV
            {
                get { return (flags & (1 << 7)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 7)); }
            }
            public bool hasMaterialIDs
            {
                get { return (flags & (1 << 8)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 8)); }
            }
            public bool hasBones
            {
                get { return (flags & (1 << 9)) != 0; }
                set { SwitchBits(ref flags, value, (1 << 9)); }
            }
        };

        public struct TRS
        {
            public Vector3 position;
            public Quaternion rotation;
            public Vector3 rotation_eularZXY;
            public Vector3 scale;
        };

        public struct TransformData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern TransformData msTransformCreate();
        }

        public struct CameraData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern CameraData msCameraCreate();
        }

        public struct MeshData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern MeshData msMeshCreate();
            [DllImport("MeshSyncServer")] static extern int msMeshGetID(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMeshSetID(IntPtr _this, int v);
            [DllImport("MeshSyncServer")] static extern MeshDataFlags msMeshGetFlags(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMeshSetFlags(IntPtr _this, MeshDataFlags v);
            [DllImport("MeshSyncServer")] static extern IntPtr msMeshGetPath(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMeshSetPath(IntPtr _this, string v);
            [DllImport("MeshSyncServer")] static extern int msMeshGetNumPoints(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msMeshGetNumIndices(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msMeshGetNumSplits(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern void msMeshReadPoints(IntPtr _this, Vector3[] dst);
            [DllImport("MeshSyncServer")] static extern void msMeshWritePoints(IntPtr _this, Vector3[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadNormals(IntPtr _this, Vector3[] dst);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteNormals(IntPtr _this, Vector3[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadTangents(IntPtr _this, Vector4[] dst);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteTangents(IntPtr _this, Vector4[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadUV(IntPtr _this, Vector2[] dst);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteUV(IntPtr _this, Vector2[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshReadIndices(IntPtr _this, int[] dst);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteIndices(IntPtr _this, int[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteSubmeshTriangles(IntPtr _this, int[] v, int size, int materialID);

            [DllImport("MeshSyncServer")] static extern void msMeshWriteWeights4(IntPtr _this, BoneWeight[] weights, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshSetBone(IntPtr _this, string v, int i);
            [DllImport("MeshSyncServer")] static extern void msMeshWriteBindPoses(IntPtr _this, Matrix4x4[] v, int size);
            [DllImport("MeshSyncServer")] static extern void msMeshSetLocal2World(IntPtr _this, ref Matrix4x4 v);
            [DllImport("MeshSyncServer")] static extern void msMeshSetWorld2Local(IntPtr _this, ref Matrix4x4 v);

            [DllImport("MeshSyncServer")] static extern SplitData msMeshGetSplit(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern void msMeshGetTransform(IntPtr _this, ref TRS dst);
            [DllImport("MeshSyncServer")] static extern void msMeshSetTransform(IntPtr _this, ref TRS v);
            [DllImport("MeshSyncServer")] static extern int msMeshGetNumSubmeshes(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern SubmeshData msMeshGetSubmesh(IntPtr _this, int i);


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

            public int id
            {
                get { return msMeshGetID(_this); }
                set { msMeshSetID(_this, value); }
            }
            public MeshDataFlags flags
            {
                get { return msMeshGetFlags(_this); }
                set { msMeshSetFlags(_this, value); }
            }
            public string path
            {
                get { return S(msMeshGetPath(_this)); }
                set { msMeshSetPath(_this, value); }
            }

            public int numPoints { get { return msMeshGetNumPoints(_this); } }
            public int numIndices { get { return msMeshGetNumIndices(_this); } }
            public int numSplits { get { return msMeshGetNumSplits(_this); } }
            public Vector3[] points
            {
                get
                {
                    var ret = new Vector3[numPoints];
                    msMeshReadPoints(_this, ret);
                    return ret;
                }
                set
                {
                    msMeshWritePoints(_this, value, value.Length);
                }
            }
            public Vector3[] normals
            {
                get
                {
                    var ret = new Vector3[numPoints];
                    msMeshReadNormals(_this, ret);
                    return ret;
                }
                set
                {
                    msMeshWriteNormals(_this, value, value.Length);
                }
            }
            public Vector4[] tangents
            {
                get
                {
                    var ret = new Vector4[numPoints];
                    msMeshReadTangents(_this, ret);
                    return ret;
                }
                set
                {
                    msMeshWriteTangents(_this, value, value.Length);
                }
            }
            public Vector2[] uv
            {
                get
                {
                    var ret = new Vector2[numPoints];
                    msMeshReadUV(_this, ret);
                    return ret;
                }
                set
                {
                    msMeshWriteUV(_this, value, value.Length);
                }
            }
            public int[] indices
            {
                get
                {
                    var ret = new int[numIndices];
                    msMeshReadIndices(_this, ret);
                    return ret;
                }
                set
                {
                    msMeshWriteIndices(_this, value, value.Length);
                }
            }
            public TRS trs
            {
                get
                {
                    var ret = default(TRS);
                    msMeshGetTransform(_this, ref ret);
                    return ret;
                }
                set
                {
                    msMeshSetTransform(_this, ref value);
                }
            }
            public Matrix4x4 local2world { set { msMeshSetLocal2World(_this, ref value); } }
            public Matrix4x4 world2local { set { msMeshSetWorld2Local(_this, ref value); } }

            public SplitData GetSplit(int i)
            {
                return msMeshGetSplit(_this, i);
            }
            public void WriteSubmeshTriangles(int[] indices, int materialID)
            {
                msMeshWriteSubmeshTriangles(_this, indices, indices.Length, materialID);
            }

            public BoneWeight[] boneWeights
            {
                set { msMeshWriteWeights4(_this, value, value.Length); }
            }
            public Matrix4x4[] bindposes
            {
                set { msMeshWriteBindPoses(_this, value, value.Length); }
            }
            public Transform[] bones
            {
                set
                {
                    int n = value.Length;
                    for(int i=0; i<n; ++i)
                    {
                        string path = BuildPath(value[i]);
                        msMeshSetBone(_this, path, i);
                    }
                }
            }

            public int numSubmeshes { get { return msMeshGetNumSubmeshes(_this); } }
            public SubmeshData GetSubmesh(int i)
            {
                return msMeshGetSubmesh(_this, i);
            }
        };

        public struct SplitData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern int msSplitGetNumPoints(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msSplitGetNumIndices(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msSplitGetNumSubmeshes(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msSplitReadPoints(IntPtr _this, Vector3[] dst);
            [DllImport("MeshSyncServer")] static extern int msSplitReadNormals(IntPtr _this, Vector3[] dst);
            [DllImport("MeshSyncServer")] static extern int msSplitReadTangents(IntPtr _this, Vector4[] dst);
            [DllImport("MeshSyncServer")] static extern int msSplitReadUV(IntPtr _this, Vector2[] dst);
            [DllImport("MeshSyncServer")] static extern int msSplitReadIndices(IntPtr _this, int[] dst);
            [DllImport("MeshSyncServer")] static extern SubmeshData msSplitGetSubmesh(IntPtr _this, int i);

            public int numPoints { get { return msSplitGetNumPoints(_this); } }
            public int numIndices { get { return msSplitGetNumIndices(_this); } }
            public int numSubmeshes { get { return msSplitGetNumSubmeshes(_this); } }
            public Vector3[] points
            {
                get
                {
                    var ret = new Vector3[numPoints];
                    msSplitReadPoints(_this, ret);
                    return ret;
                }
            }
            public Vector3[] normals
            {
                get
                {
                    var ret = new Vector3[numPoints];
                    msSplitReadNormals(_this, ret);
                    return ret;
                }
            }
            public Vector4[] tangents
            {
                get
                {
                    var ret = new Vector4[numPoints];
                    msSplitReadTangents(_this, ret);
                    return ret;
                }
            }
            public Vector2[] uv
            {
                get
                {
                    var ret = new Vector2[numPoints];
                    msSplitReadUV(_this, ret);
                    return ret;
                }
            }
            public int[] indices
            {
                get
                {
                    var ret = new int[numIndices];
                    msSplitReadIndices(_this, ret);
                    return ret;
                }
            }
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
            [DllImport("MeshSyncServer")] static extern int msSubmeshReadIndices(IntPtr _this, int[] dst);

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


        public struct ServerSettings
        {
            public int max_queue;
            public int max_threads;
            public ushort port;

            public static ServerSettings default_value
            {
                get
                {
                    return new ServerSettings
                    {
                        max_queue = 256,
                        max_threads = 8,
                        port = 8080,
                    };
                }
            }
        }

        [DllImport("MeshSyncServer")] static extern IntPtr msServerStart(ref ServerSettings settings);
        [DllImport("MeshSyncServer")] static extern void msServerStop(IntPtr sv);

        delegate void msMessageHandler(MessageType type, IntPtr data);
        [DllImport("MeshSyncServer")] static extern int msServerProcessMessages(IntPtr sv, msMessageHandler handler);

        [DllImport("MeshSyncServer")] static extern void msServerBeginServe(IntPtr sv);
        [DllImport("MeshSyncServer")] static extern void msServerEndServe(IntPtr sv);
        [DllImport("MeshSyncServer")] static extern void msServerServeMesh(IntPtr sv, MeshData data);
        [DllImport("MeshSyncServer")] static extern void msServerSetNumMaterials(IntPtr sv, int n);
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
            public GameObject go;
            public Mesh origMesh;
            public Mesh editMesh;
            public int[] materialIDs = new int[0];
            public int[] submeshCounts = new int[0];
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


        #endregion


        #region fields
        [SerializeField] int m_serverPort = 8080;
        [SerializeField] List<Material> m_materialList = new List<Material>();
        [SerializeField] string m_assetExportPath = "Assets/MeshSyncAssets";
        [SerializeField] bool m_logging = true;

        IntPtr m_server;
        msMessageHandler m_handler;
        bool m_requestRestart = false;
        bool m_requestReassignMaterials = false;
        bool m_captureScreenshotInProgress = false;

        Dictionary<string, Record> m_clientMeshes = new Dictionary<string, Record>();
        Dictionary<int, Record> m_hostMeshes = new Dictionary<int, Record>();
        Dictionary<Material, int> m_materialIDTable = new Dictionary<Material, int>();
        Dictionary<GameObject, int> m_objIDTable = new Dictionary<GameObject, int>();

        [HideInInspector][SerializeField] string[] m_clientMeshes_keys;
        [HideInInspector][SerializeField] Record[] m_clientMeshes_values;
        [HideInInspector][SerializeField] int[] m_hostMeshes_keys;
        [HideInInspector][SerializeField] Record[] m_hostMeshes_values;
        [HideInInspector][SerializeField] Material[] m_materialIDTable_keys;
        [HideInInspector][SerializeField] int[] m_materialIDTable_values;
        [HideInInspector][SerializeField] GameObject[] m_objIDTable_keys;
        [HideInInspector][SerializeField] int[] m_objIDTable_values;
        #endregion

        #region impl
        void SerializeDictionary<K,V>(Dictionary<K,V> dic, ref K[] keys, ref V[] values)
        {
            keys = dic.Keys.ToArray();
            values = dic.Values.ToArray();
        }
        void DeserializeDictionary<K, V>(Dictionary<K, V> dic, ref K[] keys, ref V[] values)
        {
            if (keys != null && values != null && keys.Length == values.Length)
            {
                int n = keys.Length;
                for (int i = 0; i < n; ++i)
                {
                    dic[keys[i]] = values[i];
                }
            }
            keys = null;
            values = null;

        }

        public void OnBeforeSerialize()
        {
            SerializeDictionary(m_clientMeshes, ref m_clientMeshes_keys, ref m_clientMeshes_values);
            SerializeDictionary(m_hostMeshes, ref m_hostMeshes_keys, ref m_hostMeshes_values);
            SerializeDictionary(m_materialIDTable, ref m_materialIDTable_keys, ref m_materialIDTable_values);
            SerializeDictionary(m_objIDTable, ref m_objIDTable_keys, ref m_objIDTable_values);
        }
        public void OnAfterDeserialize()
        {
            DeserializeDictionary(m_clientMeshes, ref m_clientMeshes_keys, ref m_clientMeshes_values);
            DeserializeDictionary(m_hostMeshes, ref m_hostMeshes_keys, ref m_hostMeshes_values);
            DeserializeDictionary(m_materialIDTable, ref m_materialIDTable_keys, ref m_materialIDTable_values);
            DeserializeDictionary(m_objIDTable, ref m_objIDTable_keys, ref m_objIDTable_values);
        }



        void StartServer()
        {
            StopServer();

            var settings = ServerSettings.default_value;
            settings.port = (ushort)m_serverPort;
            m_server = msServerStart(ref settings);
            m_handler = OnServerMessage;
#if UNITY_EDITOR
            EditorApplication.update += PollServerEvents;
#endif
        }

        void StopServer()
        {
            if(m_server != IntPtr.Zero)
            {
#if UNITY_EDITOR
                EditorApplication.update -= PollServerEvents;
#endif
                msServerStop(m_server);
                m_server = IntPtr.Zero;
            }
        }

        void PollServerEvents()
        {
            if(m_requestRestart)
            {
                m_requestRestart = false;
                StartServer();
            }
            if(m_requestReassignMaterials)
            {
                m_requestReassignMaterials = false;
                ReassignMaterials();
                ForceRepaint();
            }
            if(m_captureScreenshotInProgress)
            {
                m_captureScreenshotInProgress = false;
                msServerSetScreenshotFilePath(m_server, "screenshot.png");
            }

            if (msServerProcessMessages(m_server, m_handler) > 0)
            {
                ForceRepaint();
            }
        }

        void OnServerMessage(MessageType type, IntPtr data)
        {
            switch (type)
            {
                case MessageType.Get:
                    OnRecvGet((GetMessage)data);
                    break;
                case MessageType.Set:
                    OnRecvSet((SetMessage)data);
                    break;
                case MessageType.Delete:
                    OnRecvDelete((DeleteMessage)data);
                    break;
                case MessageType.Screenshot:
                    OnRecvScreenshot(data);
                    break;
            }
        }

        void OnRecvGet(GetMessage mes)
        {

            msServerBeginServe(m_server);
            foreach (var mr in FindObjectsOfType<Renderer>())
            {
                ServeData(mr, mes);
            }
            msServerSetNumMaterials(m_server, m_materialList.Count);
            msServerEndServe(m_server);

#if UNITY_EDITOR
            Undo.RecordObject(this, "MeshSyncServer");
#endif
            //Debug.Log("MeshSyncServer: Get");
        }

        void OnRecvDelete(DeleteMessage mes)
        {
            int numTargets = mes.numTargets;
            for (int i = 0; i < numTargets; ++i)
            {
                var id = mes.GetID(i);
                var path = mes.GetPath(i);

                if (id != 0 && m_hostMeshes.ContainsKey(id))
                {
                    var rec = m_hostMeshes[id];
                    if (rec.go != null)
                    {
#if UNITY_EDITOR
                        Undo.DestroyObjectImmediate(rec.go);
                        Undo.RecordObject(this, "MeshSyncServer");
#else
                        DestroyImmediate(rec.go);
#endif
                    }
                    m_hostMeshes.Remove(id);
                }
                else if (m_clientMeshes.ContainsKey(path))
                {
                    var rec = m_clientMeshes[path];
                    if (rec.go != null)
                    {
#if UNITY_EDITOR
                        Undo.DestroyObjectImmediate(rec.go);
                        Undo.RecordObject(this, "MeshSyncServer");
#else
                        DestroyImmediate(rec.go);
#endif
                    }
                    m_clientMeshes.Remove(path);
                }
            }

            //Debug.Log("MeshSyncServer: Delete");
        }

        void OnRecvSet(SetMessage mes)
        {
            int numMeshes = mes.numMeshes;
            for (int i = 0; i < numMeshes; ++i)
            {
                UpdateMesh(mes.GetMesh(i));
            }

            int numTransforms = mes.numTransforms;
            for (int i = 0; i < numTransforms; ++i)
            {
                UpdateTransform(mes.GetTransform(i));
            }

            int numCameras = mes.numCameras;
            for (int i = 0; i < numCameras; ++i)
            {
                UpdateCamera(mes.GetCamera(i));
            }

            //Debug.Log("MeshSyncServer: Set");
        }

        void UpdateMesh(MeshData data)
        {
            var path = data.path;
            bool createdNewMesh = false;

            // find or create target object
            Record rec = null;
            if(data.id !=0 && m_hostMeshes.ContainsKey(data.id))
            {
                rec = m_hostMeshes[data.id];
                if(rec.go == null)
                {
                    m_hostMeshes.Remove(data.id);
                    rec = null;
                }
            }
            else if(m_clientMeshes.ContainsKey(path))
            {
                rec = m_clientMeshes[path];
                if (rec.go == null)
                {
                    m_clientMeshes.Remove(path);
                    rec = null;
                }
            }

            if(rec == null)
            {
                var t = FindObjectByPath(null, path, true, ref createdNewMesh);
                if(t != null)
                {
                    rec = new Record
                    {
                        go = t.gameObject,
                        recved = true,
                    };
                    m_clientMeshes[path] = rec;
                }
            }
            if (rec == null) { return; }


            var target = rec.go.GetComponent<Transform>();

            // if object is not visible, just disable and return
            if (!data.flags.visible)
            {
                target.gameObject.SetActive(false);
                for (int i = 0; ; ++i)
                {
                    var t = FindObjectByPath(null, path + "/[" + i + "]");
                    if (t == null) { break; }
                    t.gameObject.SetActive(false);
                }
                return;
            }
            target.gameObject.SetActive(true);



            // allocate material list
            bool materialsUpdated = rec.BuildMaterialData(data);
            int maxMaterialID = rec.maxMaterialID;
            while (m_materialList.Count < maxMaterialID + 1)
            {
#if UNITY_EDITOR
                var tmp = Instantiate(GetDefaultMaterial());
                tmp.name = "DefaultMaterial";
                m_materialList.Add(tmp);
#else
                m_materialList.Add(null);
#endif
            }

            var flags = data.flags;

            // update transform
            {
                var trs = data.trs;
                target.localPosition = trs.position;
                target.localRotation = trs.rotation;
                target.localScale = trs.scale;
            }

            var smr = rec.go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null)
            {
                // update skinned mesh - only when topology is not changed
                rec.editMesh = CreateEditedMesh(data, data.GetSplit(0), true, rec.origMesh);
                if(rec.editMesh == null)
                {
                    if(m_logging)
                    {
                        Debug.Log("edit for " + rec.origMesh.name + " is ignored. currently changing topology of skinned meshes is not supported.");
                    }
                }
                else
                {
                    smr.sharedMesh = rec.editMesh;
                    rec.go.SetActive(false); // 
                    rec.go.SetActive(true);  // force recalculate skinned mesh in editor. I couldn't find better way...
                }
            }
            else
            {
                // update mesh
                for (int i = 0; i < data.numSplits; ++i)
                {
                    var t = target;
                    if (i > 0)
                    {
                        t = FindObjectByPath(null, path + "/[" + i + "]", true, ref createdNewMesh);
                        t.gameObject.SetActive(true);
                    }

                    var mfilter = GetOrAddMeshComponents(t.gameObject, i > 0);
                    if (mfilter == null) { return; }

                    var split = data.GetSplit(i);
                    if (split.numPoints > 0 && split.numIndices > 0)
                    {
                        rec.editMesh = CreateEditedMesh(data, split);
                        rec.editMesh.name = i == 0 ? target.name : target.name + "[" + i + "]";
                        mfilter.sharedMesh = rec.editMesh;
                    }
                }

                int num_splits = Math.Max(1, data.numSplits);
                for (int i = num_splits; ; ++i)
                {
                    var t = FindObjectByPath(null, path + "/[" + i + "]");
                    if (t == null) { break; }
                    DestroyImmediate(t.gameObject);
                }
            }

            // assign materials if needed
            if (materialsUpdated)
            {
                AssignMaterials(rec);
            }

#if UNITY_EDITOR
            Undo.RecordObject(this, "MeshSyncServer");
#endif
        }

        Mesh CreateEditedMesh(MeshData data, SplitData split, bool noTopologyUpdate = false, Mesh prev = null)
        {
            if(noTopologyUpdate)
            {
                if(data.numPoints != prev.vertexCount) { return null; }
            }

            var mesh = noTopologyUpdate ? Instantiate<Mesh>(prev) : new Mesh();
            var flags = data.flags;
            if (flags.hasPoints)
            {
                mesh.vertices = split.points;
            }
            if (flags.hasNormals)
            {
                mesh.normals = split.normals;
            }
            if (flags.hasTangents)
            {
                mesh.tangents = split.tangents;
            }
            if (flags.hasUV)
            {
                mesh.uv = split.uv;
            }

            if(!noTopologyUpdate)
            {
                if (split.numSubmeshes == 0)
                {
                    mesh.SetIndices(split.indices, MeshTopology.Triangles, 0);
                }
                else
                {
                    mesh.subMeshCount = split.numSubmeshes;
                    for (int smi = 0; smi < split.numSubmeshes; ++smi)
                    {
                        var submesh = split.GetSubmesh(smi);
                        mesh.SetIndices(submesh.indices, MeshTopology.Triangles, smi);
                    }
                }
            }
            mesh.RecalculateBounds();
            return mesh;
        }

        void UpdateTransform(TransformData data)
        {
            // todo
        }

        void UpdateCamera(CameraData data)
        {
            // todo 
        }

        void ReassignMaterials()
        {
            foreach (var rec in m_clientMeshes)
            {
                AssignMaterials(rec.Value);
            }
            foreach (var rec in m_hostMeshes)
            {
                AssignMaterials(rec.Value);
            }
        }

        void AssignMaterials(Record rec)
        {
            if(rec.go == null) { return; }

            var materialIDs = rec.materialIDs;
            var submeshCounts = rec.submeshCounts;

            int mi = 0;
            for (int i = 0; i < submeshCounts.Length; ++i)
            {
                Renderer r = null;
                if (i == 0)
                {
                    r = rec.go.GetComponent<Renderer>();
                }
                else
                {
                    var t = rec.go.transform.FindChild("[" + i + "]");
                    if (t == null) { break; }
                    r = t.GetComponent<Renderer>();
                }

                var materials = new Material[submeshCounts[i]];
                for (int j = 0; j < submeshCounts[i]; ++j)
                {
                    materials[j] = m_materialList[materialIDs[mi++]];
                }

                if (r != null)
                {
                    r.sharedMaterials = materials;
                }
            }
        }

        int GetMaterialID(Material mat)
        {
            if(mat == null) { return 0; }

            int ret;
            if(m_materialIDTable.ContainsKey(mat))
            {
                ret = m_materialIDTable[mat];
            }
            else
            {
                ret = m_materialIDTable.Count + 1;
                m_materialIDTable[mat] = ret;
            }

            while (m_materialList.Count < ret + 1)
            {
#if UNITY_EDITOR
                var tmp = Instantiate(GetDefaultMaterial());
                tmp.name = "DefaultMaterial";
                m_materialList.Add(tmp);
#else
                m_materialList.Add(null);
#endif
            }
            m_materialList[ret] = mat;

            return ret;
        }

        int GetObjectlID(GameObject go)
        {
            if (go == null) { return 0; }

            int ret;
            if (m_objIDTable.ContainsKey(go))
            {
                ret = m_objIDTable[go];
            }
            else
            {
                ret = m_objIDTable.Count + 1;
                m_objIDTable[go] = ret;
            }
            return ret;
        }

        Transform FindObjectByPath(Transform parent, string path)
        {
            var names = path.Split('/');
            Transform t = null;
            foreach (var name in names)
            {
                if (name.Length == 0) { continue; }
                bool created = false;
                t = FindObjectByName(t, name, false, ref created);
                if (t == null) { break; }
            }
            return t;
        }

        Transform FindObjectByPath(Transform parent, string path, bool createIfNotExist, ref bool created)
        {
            var names = path.Split('/');
            Transform t = null;
            foreach (var name in names)
            {
                if (name.Length == 0) { continue; }
                t = FindObjectByName(t, name, createIfNotExist, ref created);
                if (t == null) { break; }
            }
            return t;
        }

        Transform FindObjectByName(Transform parent, string name, bool createIfNotExist, ref bool created)
        {
            Transform ret = null;
            if (parent == null)
            {
                var roots = UnityEngine.SceneManagement.SceneManager.GetActiveScene().GetRootGameObjects();
                foreach (var go in roots)
                {
                    if (go.name == name)
                    {
                        ret = go.GetComponent<Transform>();
                        break;
                    }
                }
            }
            else
            {
                ret = parent.FindChild(name);
            }

            if (createIfNotExist && ret == null)
            {
                var go = new GameObject();
#if UNITY_EDITOR
                Undo.RegisterCreatedObjectUndo(go, "MeshSyncServer");
#endif
                go.name = name;
                ret = go.GetComponent<Transform>();
                if (parent != null)
                {
                    ret.SetParent(parent, false);
                }
                created = true;
            }
            return ret;
        }

#if UNITY_EDITOR
        static MethodInfo s_GetBuiltinExtraResourcesMethod;
        public static Material GetDefaultMaterial()
        {
            if (s_GetBuiltinExtraResourcesMethod == null)
            {
                BindingFlags bfs = BindingFlags.NonPublic | BindingFlags.Static;
                s_GetBuiltinExtraResourcesMethod = typeof(EditorGUIUtility).GetMethod("GetBuiltinExtraResource", bfs);
            }
            return (Material)s_GetBuiltinExtraResourcesMethod.Invoke(null, new object[] { typeof(Material), "Default-Material.mat" });
        }
#endif

        MeshFilter GetOrAddMeshComponents(GameObject go, bool isSplit)
        {
            var smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null)
            {
                return null;
            }

            var mfilter = go.GetComponent<MeshFilter>();
            if (mfilter == null)
            {
                mfilter = go.AddComponent<MeshFilter>();
                var renderer = go.AddComponent<MeshRenderer>();
                if (isSplit)
                {
                    var parent = go.GetComponent<Transform>().parent.GetComponent<Renderer>();
                    renderer.sharedMaterials = parent.sharedMaterials;
                }
            }
            return mfilter;
        }

        void ForceRepaint()
        {
#if UNITY_EDITOR
            EditorUtility.SetDirty(this);
            SceneView.RepaintAll();
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
#endif
        }

        static string BuildPath(Transform t)
        {
            var parent = t.parent;
            if (parent != null)
            {
                return BuildPath(parent) + "/" + t.name;
            }
            else
            {
                return "/" + t.name;
            }
        }

        bool ServeData(Renderer renderer, GetMessage mes)
        {
            bool ret = false;
            Mesh origMesh = null;

            var dst = MeshData.Create();
            if (renderer.GetType() == typeof(MeshRenderer))
            {
                ret = CaptureMeshRenderer(ref dst, renderer as MeshRenderer, mes, ref origMesh);
            }
            else if (renderer.GetType() == typeof(SkinnedMeshRenderer))
            {
                ret = CaptureSkinnedMeshRenderer(ref dst, renderer as SkinnedMeshRenderer, mes, ref origMesh);
            }

            if (ret)
            {
                dst.id = GetObjectlID(renderer.gameObject);
                var trans = renderer.GetComponent<Transform>();
                var parent = trans.parent;
                if (mes.flags.getTransform)
                {
                    var tdata = default(TRS);
                    tdata.position = trans.localPosition;
                    tdata.rotation = trans.localRotation;
                    tdata.rotation_eularZXY = trans.localEulerAngles;
                    tdata.scale = trans.localScale;
                    dst.trs = tdata;
                }
                dst.local2world = trans.localToWorldMatrix;
                dst.world2local = parent != null ? parent.worldToLocalMatrix : Matrix4x4.identity;

                if (!m_hostMeshes.ContainsKey(dst.id))
                {
                    m_hostMeshes[dst.id] = new Record();
                }
                var rec = m_hostMeshes[dst.id];
                rec.go = renderer.gameObject;
                rec.origMesh = origMesh;

                dst.path = BuildPath(renderer.GetComponent<Transform>());
                msServerServeMesh(m_server, dst);
            }
            return ret;
        }

        bool CaptureMeshRenderer(ref MeshData dst, MeshRenderer mr, GetMessage mes, ref Mesh mesh)
        {
            mesh = mr.GetComponent<MeshFilter>().sharedMesh;
            if (mesh == null) return false;
            if (!mesh.isReadable)
            {
                Debug.LogWarning("Mesh " + mr.name + " is not readable and be ignored");
                return false;
            }

            CaptureMesh(ref dst, mesh, null, mes.flags, mr.sharedMaterials);
            return true;
        }

        bool CaptureSkinnedMeshRenderer(ref MeshData dst, SkinnedMeshRenderer smr, GetMessage mes, ref Mesh mesh)
        {
            mesh = smr.sharedMesh;
            if (mesh == null) return false;
            if (!mes.bakeSkin && !mesh.isReadable)
            {
                Debug.LogWarning("Mesh " + smr.name + " is not readable and be ignored");
                return false;
            }

            Cloth cloth = smr.GetComponent<Cloth>();
            if(cloth != null && mes.bakeCloth)
            {
                CaptureMesh(ref dst, mesh, cloth, mes.flags, smr.sharedMaterials);
            }
            if (mes.bakeSkin)
            {
                var tmp = new Mesh();
                smr.BakeMesh(tmp);
                CaptureMesh(ref dst, tmp, null, mes.flags, smr.sharedMaterials);
            }
            else
            {
                CaptureMesh(ref dst, mesh, null, mes.flags, smr.sharedMaterials);
            }
            if(mes.flags.getBones)
            {
                dst.bones = smr.bones;
            }
            return true;
        }

        void CaptureMesh(ref MeshData data, Mesh mesh, Cloth cloth, GetFlags flags, Material[] materials)
        {
            bool use_cloth = cloth != null;

            if (flags.getPoints)
            {
                data.points = use_cloth ? cloth.vertices : mesh.vertices;
            }
            if (flags.getNormals)
            {
                data.normals = use_cloth ? cloth.normals : mesh.normals;
            }
            if (flags.getTangents)
            {
                data.tangents = mesh.tangents;
            }
            if (flags.getUV)
            {
                data.uv = mesh.uv;
            }
            if (flags.getIndices)
            {
                if(!flags.getMaterialIDs || materials == null || materials.Length == 0)
                {
                    data.indices = mesh.triangles;
                }
                else
                {
                    int n = mesh.subMeshCount;
                    for (int i = 0; i < n; ++i)
                    {
                        var indices = mesh.GetIndices(i);
                        int mid = i < materials.Length ? GetMaterialID(materials[i]) : 0;
                        data.WriteSubmeshTriangles(indices, mid);
                    }
                }
            }
            if (flags.getBones)
            {
                data.boneWeights = mesh.boneWeights;
                data.bindposes = mesh.bindposes;
            }
        }

        void OnRecvScreenshot(IntPtr data)
        {
            Application.CaptureScreenshot("screenshot.png");
            // actual capture will be done at end of frame. not done immediately.
            // just set flag now.
            m_captureScreenshotInProgress = true;
        }

#if UNITY_EDITOR
        public void GenerateLightmapUV(GameObject go)
        {
            if (go == null) { return; }
            var mf = go.GetComponent<MeshFilter>();
            if (mf != null)
            {
                var mesh = mf.sharedMesh;
                if (mesh != null)
                {
                    Unwrapping.GenerateSecondaryUVSet(mesh);
                }
            }
            for (int i = 1; ; ++i)
            {
                var t = go.transform.FindChild("[" + i + "]");
                if (t == null) { break; }
                GenerateLightmapUV(t.gameObject);
            }
        }
        public void GenerateLightmapUV()
        {
            foreach (var kvp in m_clientMeshes)
            {
                GenerateLightmapUV(kvp.Value.go);
            }
            foreach (var kvp in m_hostMeshes)
            {
                if(kvp.Value.editMesh != null)
                    GenerateLightmapUV(kvp.Value.go);
            }
        }

        public void ExportMeshes(GameObject go)
        {
            if(go == null) { return; }
            AssetDatabase.CreateFolder(".", m_assetExportPath);
            var mf = go.GetComponent<MeshFilter>();
            if (mf != null && mf.sharedMesh != null)
            {
                var path = m_assetExportPath + "/" + mf.sharedMesh.name + ".asset";
                AssetDatabase.CreateAsset(mf.sharedMesh, path);
                if (m_logging)
                {
                    Debug.Log("exported mesh " + path);
                }

                for (int i=1; ; ++i)
                {
                    var t = go.transform.FindChild("[" + i + "]");
                    if(t == null) { break; }
                    ExportMeshes(t.gameObject);
                }
            }
        }
        public void ExportMeshes()
        {
            // export client meshes
            foreach (var kvp in m_clientMeshes)
            {
                if(kvp.Value.go == null || !kvp.Value.go.activeInHierarchy) { continue; }
                if (kvp.Value.editMesh != null)
                {
                    ExportMeshes(kvp.Value.go);
                    kvp.Value.editMesh = null;
                }
            }

            // replace existing meshes
            int n = 0;
            foreach (var kvp in m_hostMeshes)
            {
                if (kvp.Value.go == null || !kvp.Value.go.activeInHierarchy) { continue; }
                if (kvp.Value.editMesh != null)
                {
                    kvp.Value.origMesh.Clear(); // make editor can recognize mesh has modified by CopySerialized()
                    EditorUtility.CopySerialized(kvp.Value.editMesh, kvp.Value.origMesh);
                    kvp.Value.editMesh = null;

                    var mf = kvp.Value.go.GetComponent<MeshFilter>();
                    if(mf != null) { mf.sharedMesh = kvp.Value.origMesh; }

                    var smr = kvp.Value.go.GetComponent<SkinnedMeshRenderer>();
                    if (smr != null) { smr.sharedMesh = kvp.Value.origMesh; }

                    if (m_logging)
                    {
                        Debug.Log("updated mesh " + kvp.Value.origMesh.name);
                    }

                    ++n;
                }
            }
            if (n > 0)
            {
                AssetDatabase.SaveAssets();
            }
        }
#endif


#if UNITY_EDITOR
        void Reset()
        {
            // force disable batching for export
            var method = typeof(UnityEditor.PlayerSettings).GetMethod("SetBatchingForPlatform", BindingFlags.NonPublic | BindingFlags.Static);
            if (method != null)
            {
                method.Invoke(null, new object[] { BuildTarget.StandaloneWindows, 0, 0 });
                method.Invoke(null, new object[] { BuildTarget.StandaloneWindows64, 0, 0 });
            }
        }

        void OnValidate()
        {
            m_requestRestart = true;
            m_requestReassignMaterials = true;
        }
#endif

        void Awake()
        {
            StartServer();
        }

        void OnDestroy()
        {
            StopServer();
        }


#if UNITY_EDITOR
        bool m_isCompiling;
#endif

        void Update()
        {
#if UNITY_EDITOR
            if (EditorApplication.isCompiling && !m_isCompiling)
            {
                // on compile begin
                m_isCompiling = true;
                StopServer();
            }
            else if (!EditorApplication.isCompiling && m_isCompiling)
            {
                // on compile end
                m_isCompiling = false;
                StartServer();
            }
#endif
            PollServerEvents();
        }
        #endregion

        void ErrorOnStandaloneBuild()
        {
            // error on standalone build - on purpose.
            // this plugin is intended to use only on editor. if you really want to use on runtime. comment out next line.
            EditorUtility.SetDirty(this);
        }
    }
}
