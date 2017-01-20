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
        }
#endif

        public enum EventType
        {
            Unknown,
            Get,
            Delete,
            Mesh,
        }

        public struct GetFlags
        {
            public int flags;
            public bool getTransform { get { return (flags & 0x1) != 0; } }
            public bool getPoints { get { return (flags & 0x2) != 0; } }
            public bool getNormals { get { return (flags & 0x4) != 0; } }
            public bool getTangents { get { return (flags & 0x8) != 0; } }
            public bool getUV { get { return (flags & 0x10) != 0; } }
            public bool getIndices { get { return (flags & 0x20) != 0; } }
            public bool getBones { get { return (flags & 0x40) != 0; } }
            public bool swapHandedness { get { return (flags & 0x80) != 0; } }
            public bool swapFaces { get { return (flags & 0x100) != 0; } }
            public bool bakeSkin { get { return (flags & 0x200) != 0; } }
            public bool applyLocal2world { get { return (flags & 0x400) != 0; } }
            public bool applyWorld2local { get { return (flags & 0x800) != 0; } }
        }

        public struct GetData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern GetFlags msGetGetFlags(IntPtr _this);

            public static explicit operator GetData(IntPtr v)
            {
                GetData ret;
                ret._this = v;
                return ret;
            }

            public GetFlags flags { get { return msGetGetFlags(_this); } }
        }

        public struct DeleteData
        {
            internal IntPtr _this;
            [DllImport("MeshSyncServer")] static extern IntPtr msDeleteGetPath(IntPtr _this);
            [DllImport("MeshSyncServer")] static extern int msDeleteGetID(IntPtr _this);

            public static explicit operator DeleteData(IntPtr v)
            {
                DeleteData ret;
                ret._this = v;
                return ret;
            }

            public string path { get { return S(msDeleteGetPath(_this)); } }
            public int id { get { return msDeleteGetID(_this); } }
        }


        public struct MeshDataFlags
        {
            public int flags;
            public bool visible
            {
                get { return (flags & 0x1) != 0; }
                set { SwitchBits(ref flags, value, 0x1); }
            }
            public bool hasIndices
            {
                get { return (flags & 0x2) != 0; }
                set { SwitchBits(ref flags, value, 0x2); }
            }
            public bool hasCounts
            {
                get { return (flags & 0x4) != 0; }
                set { SwitchBits(ref flags, value, 0x4); }
            }
            public bool hasPoints
            {
                get { return (flags & 0x8) != 0; }
                set { SwitchBits(ref flags, value, 0x8); }
            }
            public bool hasNormals
            {
                get { return (flags & 0x10) != 0; }
                set { SwitchBits(ref flags, value, 0x10); }
            }
            public bool hasTangents
            {
                get { return (flags & 0x20) != 0; }
                set { SwitchBits(ref flags, value, 0x20); }
            }
            public bool hasUV
            {
                get { return (flags & 0x40) != 0; }
                set { SwitchBits(ref flags, value, 0x40); }
            }
            public bool hasMaterialIDs
            {
                get { return (flags & 0x80) != 0; }
                set { SwitchBits(ref flags, value, 0x80); }
            }
            public bool hasTransform
            {
                get { return (flags & 0x100) != 0; }
                set { SwitchBits(ref flags, value, 0x100); }
            }
            public bool hasRefineSettings
            {
                get { return (flags & 0x200) != 0; }
                set { SwitchBits(ref flags, value, 0x200); }
            }
        };


        public struct TransformData
        {
            public Vector3 position;
            public Quaternion rotation;
            public Vector3 rotation_eularZXY;
            public Vector3 scale;
            public Matrix4x4 local2world;
            public Matrix4x4 world2local;
        };

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
            [DllImport("MeshSyncServer")] static extern SplitData msMeshGetSplit(IntPtr _this, int i);
            [DllImport("MeshSyncServer")] static extern void msMeshGetTransform(IntPtr _this, ref TransformData dst);
            [DllImport("MeshSyncServer")] static extern void msMeshSetTransform(IntPtr _this, ref TransformData v);


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
            public TransformData transform
            {
                get
                {
                    var ret = default(TransformData);
                    msMeshGetTransform(_this, ref ret);
                    return ret;
                }
                set
                {
                    msMeshSetTransform(_this, ref value);
                }
            }
            public SplitData GetSplit(int i)
            {
                return msMeshGetSplit(_this, i);
            }
            public void WriteSubmeshTriangles(int[] indices, int materialID)
            {
                msMeshWriteSubmeshTriangles(_this, indices, indices.Length, materialID);
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

        delegate void msMessageHandler(EventType type, IntPtr data);
        [DllImport("MeshSyncServer")] static extern int msServerProcessMessages(IntPtr sv, msMessageHandler handler);

        [DllImport("MeshSyncServer")] static extern void msServerBeginServe(IntPtr sv);
        [DllImport("MeshSyncServer")] static extern void msServerEndServe(IntPtr sv);
        [DllImport("MeshSyncServer")] static extern void msServerAddServeData(IntPtr sv, EventType et, MeshData data);
        
        static int[][] GetMaterialIDs(MeshData mdata, ref int maxID)
        {
            maxID = 0;
            int[][] ret = new int[mdata.numSplits][];
            for (int si = 0; si < ret.Length; ++si)
            {
                var split = mdata.GetSplit(si);
                var ids = new int[split.numSubmeshes];
                ret[si] = ids;
                for (int smi = 0; smi < split.numSubmeshes; ++smi)
                {
                    int mid = split.GetSubmesh(smi).materialID;
                    ids[smi] = mid;
                    maxID = Math.Max(maxID, mid);
                }
            }

            return ret;
        }

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
            public int[][] materialIDs;
            public bool recved = false;
            public bool edited = false;

            public bool CompareMaterialIDs(int[][] v)
            {
                if(materialIDs.Length != v.Length) { return false; }
                for(int i=0; i< materialIDs.Length; ++i)
                {
                    if(!materialIDs[i].SequenceEqual(v[i])) { return false; }
                }
                return false;
            }
        }


        #endregion


        #region fields
        [SerializeField] int m_serverPort = 8080;
        [SerializeField] string m_assetPath = "Assets/MeshSyncAssets";
        [SerializeField] Material[] m_materialList;

        IntPtr m_server;
        msMessageHandler m_handler;
        bool m_requestRestart = false;
        bool m_requestReassignMaterials = false;

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
        void SerializeDictionary<K,V>(Dictionary<K,V> dic, K[] keys, V[] values)
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
            SerializeDictionary(m_clientMeshes, m_clientMeshes_keys, m_clientMeshes_values);
            SerializeDictionary(m_hostMeshes, m_hostMeshes_keys, m_hostMeshes_values);
            SerializeDictionary(m_materialIDTable, m_materialIDTable_keys, m_materialIDTable_values);
            SerializeDictionary(m_objIDTable, m_objIDTable_keys, m_objIDTable_values);
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
            m_handler = OnServerEvent;
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

            if (msServerProcessMessages(m_server, m_handler) > 0)
            {
                ForceRepaint();
            }
        }

        void OnServerEvent(EventType type, IntPtr data)
        {
            switch (type)
            {
                case EventType.Get:
                    OnRecvGet((GetData)data);
                    break;
                case EventType.Delete:
                    OnRecvDelete((DeleteData)data);
                    break;
                case EventType.Mesh:
                    OnRecvMesh((MeshData)data);
                    break;
            }
        }

        void OnRecvGet(GetData data)
        {

            msServerBeginServe(m_server);
            foreach (var mr in FindObjectsOfType<Renderer>())
            {
                ServeData(mr, data.flags);
            }
            msServerEndServe(m_server);

            //Debug.Log("MeshSyncServer: Get");
        }

        void OnRecvDelete(DeleteData data)
        {
            var id = data.id;
            var path = data.path;

            if (id != 0 && m_hostMeshes.ContainsKey(id))
            {
                var rec = m_hostMeshes[id];
                if (rec.go != null)
                {
                    DestroyImmediate(rec.go);
                }
                m_hostMeshes.Remove(id);
            }
            else if (m_clientMeshes.ContainsKey(path))
            {
                var rec = m_clientMeshes[path];
                if (rec.go != null)
                {
                    DestroyImmediate(rec.go);
                }
                m_clientMeshes.Remove(path);
            }

            //Debug.Log("MeshSyncServer: Delete " + path);
        }

        void OnRecvMesh(MeshData data)
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


            // currently editing skinned mesh is not supported
            if (rec.go.GetComponent<SkinnedMeshRenderer>() != null)
            {
                return;
            }

            var target = rec.go.GetComponent<Transform>();
            rec.edited = true;


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
            int maxMaterialID = 0;
            var materialIDs = GetMaterialIDs(data, ref maxMaterialID);
            bool materialsUpdated = rec.recved && (rec.materialIDs == null || !rec.CompareMaterialIDs(materialIDs));
            rec.materialIDs = materialIDs;
            if(m_materialList == null || maxMaterialID + 1 > m_materialList.Length)
            {
                var tmp = new Material[maxMaterialID + 1];
                if(m_materialList != null)
                {
                    Array.Copy(m_materialList, tmp, Math.Min(m_materialList.Length, maxMaterialID));
                }
#if UNITY_EDITOR
                for (int i = m_materialList != null ? m_materialList.Length : 0; i < tmp.Length; ++i)
                {
                    var mat = Instantiate(GetDefaultMaterial());
                    mat.name = "DefaultMaterial";
                    tmp[i] = mat;
                }
#endif
                m_materialList = tmp;
            }

            var flags = data.flags;

            // update transform
            if (flags.hasTransform)
            {
                target.localPosition = data.transform.position;
                target.localRotation = data.transform.rotation;
                target.localScale = data.transform.scale;
            }

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
                if(mfilter == null) { return; }
                var mesh = new Mesh();
                mesh.name = i == 0 ? target.name : target.name + "[" + i + "]";

                var split = data.GetSplit(i);
                if (split.numPoints > 0 && split.numIndices > 0)
                {
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
                    mfilter.sharedMesh = mesh;
                }
            }

            int num_splits = Math.Max(1, data.numSplits);
            for (int i = num_splits; ; ++i)
            {
                var t = FindObjectByPath(null, path + "/[" + i + "]");
                if (t == null) { break; }
                DestroyImmediate(t.gameObject);
            }


            // assign materials if needed
            if (materialsUpdated)
            {
                AssignMaterials(rec);
            }

#if UNITY_EDITOR
            EditorUtility.SetDirty(target.gameObject);
#endif

            //Debug.Log("MeshSyncServer: Mesh " + path);
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
            if(rec.go == null || !rec.edited) { return; }

            var mids = rec.materialIDs;
            var mlist = new Material[mids.Length][];
            for (int i = 0; i < mids.Length; ++i)
            {
                mlist[i] = new Material[mids[i].Length];
                for (int j = 0; j < mids[i].Length; ++j)
                {
                    mlist[i][j] = m_materialList[mids[i][j]];
                }
            }

            for (int i = 0; i < mlist.Length; ++i)
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

                if (r != null)
                {
                    r.sharedMaterials = mlist[i];
                }
            }
        }

        int GetMaterialID(Material mat)
        {
            if(mat == null) { return -1; }
            if(!m_materialIDTable.ContainsKey(mat))
            {
                m_materialIDTable[mat] = m_materialIDTable.Count;
            }
            return m_materialIDTable[mat];
        }
        int GetObjectlID(GameObject go)
        {
            if (go == null) { return 0; }
            if (!m_objIDTable.ContainsKey(go))
            {
                m_objIDTable[go] = m_objIDTable.Count + 1;
            }
            return m_objIDTable[go];
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

        string GetPath(Transform t)
        {
            var parent = t.parent;
            if (parent != null)
            {
                return GetPath(parent) + "/" + t.name;
            }
            else
            {
                return "/" + t.name;
            }
        }

        bool ServeData(Renderer renderer, GetFlags flags)
        {
            bool ret = false;

            var dst = MeshData.Create();
            if (renderer.GetType() == typeof(MeshRenderer))
            {
                ret = CaptureMeshRenderer(ref dst, renderer as MeshRenderer, flags);
            }
            else if (renderer.GetType() == typeof(SkinnedMeshRenderer))
            {
                ret = CaptureSkinnedMeshRenderer(ref dst, renderer as SkinnedMeshRenderer, flags);
            }

            if (ret)
            {
                dst.id = GetObjectlID(renderer.gameObject);
                if (flags.getTransform)
                {
                    var tdata = default(TransformData);
                    CaptureTransform(ref tdata, renderer.GetComponent<Transform>());
                    dst.transform = tdata;
                }

                if(!m_hostMeshes.ContainsKey(dst.id))
                {
                    m_hostMeshes[dst.id] = new Record
                    {
                        go = renderer.gameObject,
                    };
                }

                dst.path = GetPath(renderer.GetComponent<Transform>());
                msServerAddServeData(m_server, EventType.Mesh, dst);
            }
            return ret;
        }

        bool CaptureMeshRenderer(ref MeshData dst, MeshRenderer mr, GetFlags flags)
        {
            var mesh = mr.GetComponent<MeshFilter>().sharedMesh;
            if (mesh == null) return false;
            if (!mesh.isReadable)
            {
                Debug.LogWarning("Mesh " + mr.name + " is not readable and be ignored");
                return false;
            }

            CaptureMesh(ref dst, mesh, null, flags, mr.sharedMaterials);
            return true;
        }

        bool CaptureSkinnedMeshRenderer(ref MeshData dst, SkinnedMeshRenderer smr, GetFlags flags)
        {
            if (flags.bakeSkin)
            {
                Cloth cloth = smr.GetComponent<Cloth>();
                if (cloth != null)
                {
                    var mesh = smr.sharedMesh;
                    if (mesh == null)
                    {
                        return false;
                    }
                    if (!mesh.isReadable)
                    {
                        Debug.LogWarning("Mesh " + smr.name + " is not readable and be ignored");
                        return false;
                    }
                    CaptureMesh(ref dst, mesh, cloth, flags, smr.sharedMaterials);
                }
                else
                {
                    var mesh = new Mesh();
                    smr.BakeMesh(mesh);
                    CaptureMesh(ref dst, mesh, null, flags, smr.sharedMaterials);
                }
            }
            else
            {
                var mesh = smr.sharedMesh;
                if (mesh == null) return false;
                if (!mesh.isReadable)
                {
                    Debug.LogWarning("Mesh " + smr.name + " is not readable and be ignored");
                    return false;
                }
                CaptureMesh(ref dst, mesh, null, flags, smr.sharedMaterials);
            }
            return true;
        }

        void CaptureTransform(ref TransformData data, Transform trans)
        {
            data.position = trans.localPosition;
            data.rotation = trans.localRotation;
            data.rotation_eularZXY = trans.localEulerAngles;
            data.scale = trans.localScale;
            data.local2world = trans.localToWorldMatrix;
            data.world2local = trans.worldToLocalMatrix;
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
                if(materials == null || materials.Length == 0)
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
                if(kvp.Value.edited)
                    GenerateLightmapUV(kvp.Value.go);
            }
        }

        public void ExportMeshes(GameObject go)
        {
            if(go == null) { return; }
            AssetDatabase.CreateFolder(".", m_assetPath);
            var mf = go.GetComponent<MeshFilter>();
            if (mf != null && mf.sharedMesh != null)
            {
                try
                {
                    AssetDatabase.CreateAsset(mf.sharedMesh, m_assetPath + "/" + mf.sharedMesh.name + ".asset");
                }
                catch(Exception)
                {
                }

                for(int i=1; ; ++i)
                {
                    var t = go.transform.FindChild("[" + i + "]");
                    if(t == null) { break; }
                    ExportMeshes(t.gameObject);
                }
            }
        }
        public void ExportMeshes()
        {
            foreach (var kvp in m_clientMeshes)
            {
                ExportMeshes(kvp.Value.go);
            }
            foreach (var kvp in m_hostMeshes)
            {
                if (kvp.Value.edited)
                    ExportMeshes(kvp.Value.go);
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
    }
}
