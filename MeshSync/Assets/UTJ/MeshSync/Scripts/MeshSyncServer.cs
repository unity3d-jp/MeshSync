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
    public class MeshSyncServer : MonoBehaviour
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
            public bool get_transform { get { return (flags & 0x1) != 0; } }
            public bool get_points { get { return (flags & 0x2) != 0; } }
            public bool get_normals { get { return (flags & 0x4) != 0; } }
            public bool get_tangents { get { return (flags & 0x8) != 0; } }
            public bool get_uv { get { return (flags & 0x10) != 0; } }
            public bool get_indices { get { return (flags & 0x20) != 0; } }
            public bool get_bones { get { return (flags & 0x40) != 0; } }
            public bool swap_handedness { get { return (flags & 0x80) != 0; } }
            public bool swap_faces { get { return (flags & 0x100) != 0; } }
            public bool bake_skin { get { return (flags & 0x200) != 0; } }
            public bool apply_local2world { get { return (flags & 0x400) != 0; } }
            public bool apply_world2local { get { return (flags & 0x800) != 0; } }
        }

        public struct GetData
        {
            public GetFlags flags;
        }

        public struct DeleteData
        {
            public IntPtr path;
            public int id;
        }


        public struct MeshDataFlags
        {
            public int flags;
            public bool visible
            {
                get { return (flags & 0x1) != 0; }
                set { SwitchBits(ref flags, value, 0x1); }
            }
            public bool has_indices
            {
                get { return (flags & 0x2) != 0; }
                set { SwitchBits(ref flags, value, 0x2); }
            }
            public bool has_counts
            {
                get { return (flags & 0x4) != 0; }
                set { SwitchBits(ref flags, value, 0x4); }
            }
            public bool has_points
            {
                get { return (flags & 0x8) != 0; }
                set { SwitchBits(ref flags, value, 0x8); }
            }
            public bool has_normals
            {
                get { return (flags & 0x10) != 0; }
                set { SwitchBits(ref flags, value, 0x10); }
            }
            public bool has_tangents
            {
                get { return (flags & 0x20) != 0; }
                set { SwitchBits(ref flags, value, 0x20); }
            }
            public bool has_uv
            {
                get { return (flags & 0x40) != 0; }
                set { SwitchBits(ref flags, value, 0x40); }
            }
            public bool has_materialIDs
            {
                get { return (flags & 0x80) != 0; }
                set { SwitchBits(ref flags, value, 0x80); }
            }
            public bool has_transform
            {
                get { return (flags & 0x100) != 0; }
                set { SwitchBits(ref flags, value, 0x100); }
            }
            public bool has_refine_settings
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
            public IntPtr cpp;
            public int id;
            public MeshDataFlags flags;
            public IntPtr path;
            public IntPtr points;
            public IntPtr normals;
            public IntPtr tangents;
            public IntPtr uv;
            public IntPtr indices;
            public int num_points;
            public int num_indices;
            public int num_splits;
            public TransformData transform;
        };
        public struct SplitData
        {
            public IntPtr points;
            public IntPtr normals;
            public IntPtr tangents;
            public IntPtr uv;
            public IntPtr indices;
            public IntPtr submeshes;
            public int num_points;
            public int num_indices;
            public int num_submeshes;
        };

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

        [DllImport("MeshSyncServer")] public static extern IntPtr msServerStart(ref ServerSettings settings);
        [DllImport("MeshSyncServer")] public static extern void msServerStop(IntPtr sv);

        public delegate void msMessageHandler(EventType type, IntPtr data);
        [DllImport("MeshSyncServer")] public static extern int msServerProcessMessages(IntPtr sv, msMessageHandler handler);

        [DllImport("MeshSyncServer")] public static extern void msServerBeginServe(IntPtr sv);
        [DllImport("MeshSyncServer")] public static extern void msServerEndServe(IntPtr sv);
        [DllImport("MeshSyncServer")] public static extern void msServerAddServeData(IntPtr sv, EventType et, ref MeshData data);

        [DllImport("MeshSyncServer")] public static extern void msCopyData(EventType et, ref GetData dst, IntPtr src);
        [DllImport("MeshSyncServer")] public static extern void msCopyData(EventType et, ref DeleteData dst, IntPtr src);
        [DllImport("MeshSyncServer")] public static extern void msCopyData(EventType et, ref MeshData dst, ref MeshData src);
        [DllImport("MeshSyncServer")] public static extern void msCopyData(EventType et, ref MeshData dst, IntPtr src);

        [DllImport("MeshSyncServer")] public static extern IntPtr msCreateString(string str);
        [DllImport("MeshSyncServer")] public static extern void msDeleteString(IntPtr str);

        [DllImport("MeshSyncServer")] public static extern void msGetSplitData(ref SplitData dst, ref MeshData v, int i);
        [DllImport("MeshSyncServer")] public static extern void msCopySplitData(ref SplitData dst, ref SplitData src);
        [DllImport("MeshSyncServer")] public static extern int  msSplitGetMaterialID(ref SplitData src, int i);
        [DllImport("MeshSyncServer")] public static extern int  msSplitGetNumIndices(ref SplitData src, int i);
        [DllImport("MeshSyncServer")] public static extern void msSplitCopyIndices(IntPtr dst, ref SplitData src, int i);

        static int[][] GetMaterialIDs(ref MeshData mdata, ref int maxID)
        {
            maxID = 0;
            int[][] ret = new int[mdata.num_splits][];
            var split = default(SplitData);
            for (int si = 0; si < mdata.num_splits; ++si)
            {
                msGetSplitData(ref split, ref mdata, si);
                var ids = new int[split.num_submeshes];
                ret[si] = ids;
                for (int mi = 0; mi < split.num_submeshes; ++mi)
                {
                    int mid = msSplitGetMaterialID(ref split, mi);
                    ids[mi] = mid;
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
        [SerializeField] int m_port = 8080;
        [SerializeField] bool m_genLightmapUV = false;
        [SerializeField] Material[] m_materials;

        IntPtr m_server;
        msMessageHandler m_handler;

        Vector3[] m_points;
        Vector3[] m_normals;
        Vector4[] m_tangents;
        Vector2[] m_uv;
        int[] m_indices;
        bool m_requestRestart = false;
        bool m_requestReassignMaterials = false;

        Dictionary<string, Record> m_recvedObjects = new Dictionary<string, Record>();
        Dictionary<int, Record> m_sentObjects = new Dictionary<int, Record>();
        #endregion



        #region impl
        void StartServer()
        {
            StopServer();

            var settings = ServerSettings.default_value;
            settings.port = (ushort)m_port;
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
                    OnRecvGet(data);
                    break;
                case EventType.Delete:
                    OnRecvDelete(data);
                    break;
                case EventType.Mesh:
                    OnRecvMesh(data);
                    break;
            }
        }

        void OnRecvGet(IntPtr data)
        {
            var edata = default(GetData);
            msCopyData(EventType.Get, ref edata, data);

            msServerBeginServe(m_server);
            foreach (var mr in FindObjectsOfType<Renderer>())
            {
                ServeData(mr, edata.flags);
            }
            msServerEndServe(m_server);

            //Debug.Log("MeshSyncServer: Get");
        }

        void OnRecvDelete(IntPtr data)
        {
            var edata = default(DeleteData);
            msCopyData(EventType.Delete, ref edata, data);
            var path = S(edata.path);

            if (edata.id != 0 && m_sentObjects.ContainsKey(edata.id))
            {
                var rec = m_sentObjects[edata.id];
                if (rec.go != null)
                {
                    DestroyImmediate(rec.go);
                }
                m_sentObjects.Remove(edata.id);
            }
            else if (m_recvedObjects.ContainsKey(path))
            {
                var rec = m_recvedObjects[path];
                if (rec.go != null)
                {
                    DestroyImmediate(rec.go);
                }
                m_recvedObjects.Remove(path);
            }

            //Debug.Log("MeshSyncServer: Delete " + path);
        }

        void OnRecvMesh(IntPtr data)
        {
            var edata = default(MeshData);
            msCopyData(EventType.Mesh, ref edata, data);
            var path = S(edata.path);
            bool createdNewMesh = false;

            // find or create target object
            Record rec = null;
            if(edata.id !=0 && m_sentObjects.ContainsKey(edata.id))
            {
                rec = m_sentObjects[edata.id];
                if(rec.go == null)
                {
                    m_sentObjects.Remove(edata.id);
                    rec = null;
                }
            }
            else if(m_recvedObjects.ContainsKey(path))
            {
                rec = m_recvedObjects[path];
                if (rec.go == null)
                {
                    m_recvedObjects.Remove(path);
                }
            }

            if(rec == null)
            {
                var t = FindObjectByPath(null, path, true, ref createdNewMesh);
                rec = new Record
                {
                    go = t.gameObject,
                    recved = true,
                };
                m_recvedObjects[path] = rec;
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
            if (!edata.flags.visible)
            {
                target.gameObject.SetActive(false);
                for (int i = 0; ; ++i)
                {
                    var t = FindObjectByPath(null, path + "/Submesh[" + i + "]");
                    if (t == null) { break; }
                    t.gameObject.SetActive(false);
                }
                return;
            }
            target.gameObject.SetActive(true);


            // allocate material list
            int maxMaterialID = 0;
            var materialIDs = GetMaterialIDs(ref edata, ref maxMaterialID);
            bool materialsUpdated = rec.recved && (rec.materialIDs == null || !rec.CompareMaterialIDs(materialIDs));
            rec.materialIDs = materialIDs;
            if(m_materials == null || maxMaterialID + 1 > m_materials.Length)
            {
                var tmp = new Material[maxMaterialID + 1];
                if(m_materials != null)
                {
                    Array.Copy(m_materials, tmp, Math.Min(m_materials.Length, maxMaterialID));
#if UNITY_EDITOR
                    for (int i = m_materials.Length; i < tmp.Length; ++i)
                    {
                        var mat = Instantiate(GetDefaultMaterial());
                        mat.name = "DefaultMaterial";
                        tmp[i] = mat;
                    }
#endif
                }
                m_materials = tmp;
            }


            // update transform
            if (edata.flags.has_transform)
            {
                target.localPosition = edata.transform.position;
                target.localRotation = edata.transform.rotation;
                target.localScale = edata.transform.scale;
            }

            // update mesh
            for (int i = 0; i < edata.num_splits; ++i)
            {
                var smd = default(SplitData);
                msGetSplitData(ref smd, ref edata, i);

                var mdata = default(SplitData);
                if (smd.points != IntPtr.Zero)
                {
                    m_points = new Vector3[smd.num_points];
                    mdata.points = RawPtr(m_points);
                }
                if (smd.normals != IntPtr.Zero)
                {
                    m_normals = new Vector3[smd.num_points];
                    mdata.normals = RawPtr(m_normals);
                }
                if (smd.tangents != IntPtr.Zero)
                {
                    m_tangents = new Vector4[smd.num_points];
                    mdata.tangents = RawPtr(m_tangents);
                }
                if (smd.uv != IntPtr.Zero)
                {
                    m_uv = new Vector2[smd.num_points];
                    mdata.uv = RawPtr(m_uv);
                }
                msCopySplitData(ref mdata, ref smd);

                var t = target;
                if (i > 0)
                {
                    t = FindObjectByPath(null, path + "/Submesh[" + i + "]", true, ref createdNewMesh);
                    t.gameObject.SetActive(true);
                }

                var mesh = GetOrAddMeshComponents(t.gameObject);
                if (!mesh.isReadable) { return; }
                mesh.Clear();
                if (mdata.num_points > 0 && mdata.num_indices > 0)
                {
                    if (mdata.points != IntPtr.Zero) { mesh.vertices = m_points; }
                    if (mdata.normals != IntPtr.Zero) { mesh.normals = m_normals; }
                    if (mdata.tangents != IntPtr.Zero) { mesh.tangents = m_tangents; }
                    if (mdata.uv != IntPtr.Zero) { mesh.uv = m_uv; }

                    if (mdata.num_submeshes == 0 && smd.indices != IntPtr.Zero)
                    {
                        var tmp = default(SplitData);
                        m_indices = new int[smd.num_indices];
                        tmp.indices = RawPtr(m_indices);
                        msCopySplitData(ref tmp, ref smd);
                        mesh.subMeshCount = 1;
                        mesh.SetIndices(m_indices, MeshTopology.Triangles, 0);
                    }
                    else
                    {
                        mesh.subMeshCount = mdata.num_submeshes;
                        for (int smi = 0; smi < mdata.num_submeshes; ++smi)
                        {
                            int num_indices = msSplitGetNumIndices(ref smd, smi);
                            m_indices = new int[num_indices];
                            var ptr = RawPtr(m_indices);
                            msSplitCopyIndices(ptr, ref smd, smi);
                            mesh.SetIndices(m_indices, MeshTopology.Triangles, smi);
                        }
                    }
#if UNITY_EDITOR
                    if (m_genLightmapUV) Unwrapping.GenerateSecondaryUVSet(mesh);
#endif
                    mesh.UploadMeshData(false);
                }
            }

            int num_splits = Math.Max(1, edata.num_splits);
            for (int i = num_splits; ; ++i)
            {
                var t = FindObjectByPath(null, path + "/Submesh[" + i + "]");
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
            foreach (var rec in m_recvedObjects)
            {
                AssignMaterials(rec.Value);
            }
            foreach (var rec in m_sentObjects)
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
                    mlist[i][j] = m_materials[mids[i][j]];
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

                    var t = rec.go.transform.FindChild("/Submesh[" + i + "]");
                    if (t == null) { break; }
                    r = t.GetComponent<Renderer>();
                }

                if (r != null)
                {
                    r.sharedMaterials = mlist[i];
                }
            }
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

        void SyncMaterials()
        {

        }

        Mesh GetOrAddMeshComponents(GameObject go)
        {
            var smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null)
            {
                return smr.sharedMesh;
            }

            var mfilter = go.GetComponent<MeshFilter>();
            if (mfilter == null)
            {
                mfilter = go.AddComponent<MeshFilter>();
                var mesh = new Mesh();
                mesh.name = go.name;
                mfilter.sharedMesh = mesh;
                mesh.MarkDynamic();

                var renderer = go.AddComponent<MeshRenderer>();
                if (go.name.StartsWith("Submesh"))
                {
                    var parent = go.GetComponent<Transform>().parent.GetComponent<Renderer>();
                    renderer.sharedMaterials = parent.sharedMaterials;
                }
            }
            return mfilter.sharedMesh;
        }

        void ForceRepaint()
        {
#if UNITY_EDITOR
            EditorUtility.SetDirty(this);
            SceneView.RepaintAll();
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

            var dst = default(MeshData);
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
                dst.id = renderer.gameObject.GetInstanceID();
                dst.flags.visible = true;
                if (flags.get_transform)
                {
                    dst.flags.has_transform = true;
                    CaptureTransform(ref dst.transform, renderer.GetComponent<Transform>());
                }

                int id = renderer.gameObject.GetInstanceID();
                if(!m_sentObjects.ContainsKey(id))
                {
                    m_sentObjects[id] = new Record
                    {
                        go = renderer.gameObject,
                    };
                }

                dst.path = msCreateString(GetPath(renderer.GetComponent<Transform>()));
                msServerAddServeData(m_server, EventType.Mesh, ref dst);
                msDeleteString(dst.path);
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

            CaptureMesh(ref dst, mesh, null, flags);
            return true;
        }

        bool CaptureSkinnedMeshRenderer(ref MeshData dst, SkinnedMeshRenderer smr, GetFlags flags)
        {
            if (flags.bake_skin)
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
                    CaptureMesh(ref dst, mesh, cloth, flags);
                }
                else
                {
                    var mesh = new Mesh();
                    smr.BakeMesh(mesh);
                    CaptureMesh(ref dst, mesh, null, flags);
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
                CaptureMesh(ref dst, mesh, null, flags);
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

        void CaptureMesh(ref MeshData data, Mesh mesh, Cloth cloth, GetFlags flags)
        {
            bool use_cloth = cloth != null;

            if (flags.get_points)
            {
                m_points = use_cloth ? cloth.vertices : mesh.vertices;
                data.points = RawPtr(m_points);
                data.num_points = m_points.Length;
                data.flags.has_points = m_points != null && m_points.Length > 0;
            }
            if (flags.get_normals)
            {
                m_normals = use_cloth ? cloth.normals : mesh.normals;
                data.normals = RawPtr(m_normals);
                data.num_points = m_normals.Length;
                data.flags.has_normals = m_normals != null && m_normals.Length > 0;
            }
            if (flags.get_tangents)
            {
                m_tangents = mesh.tangents;
                data.tangents = RawPtr(m_tangents);
                data.num_points = m_tangents.Length;
                data.flags.has_tangents = m_tangents != null && m_tangents.Length > 0;
            }
            if (flags.get_uv)
            {
                m_uv = mesh.uv;
                data.uv = RawPtr(m_uv);
                data.num_points = m_uv.Length;
                data.flags.has_uv = m_uv != null && m_uv.Length > 0;
            }
            if (flags.get_indices)
            {
                m_indices = mesh.triangles;
                data.indices = RawPtr(m_indices);
                data.num_indices = m_indices.Length;
                data.flags.has_indices = m_indices != null && m_indices.Length > 0;
            }
        }



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
