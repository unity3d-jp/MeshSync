using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
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
            var prefab = AssetDatabase.LoadAssetAtPath<GameObject>("Assets/UTJ/MeshSync/Prefabs/MeshSyncServer.prefab");
            if (prefab != null)
            {
                PrefabUtility.InstantiatePrefab(prefab);
            }
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
            public int id_unity;
            public int id_dcc;
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
        [DllImport("MeshSyncServer")] public static extern int  msSubmeshGetMaterialID(ref SplitData src, int i);
        [DllImport("MeshSyncServer")] public static extern int  msSubmeshGetNumIndices(ref SplitData src, int i);
        [DllImport("MeshSyncServer")] public static extern void msSubmeshCopyIndices(IntPtr dst, ref SplitData src, int i);


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
        #endregion


        #region fields
        [SerializeField] int m_port = 8080;
        [SerializeField] bool m_genLightmapUV = false;
        IntPtr m_server;
        msMessageHandler m_handler;

        Vector3[] m_points;
        Vector3[] m_normals;
        Vector4[] m_tangents;
        Vector2[] m_uv;
        int[] m_indices;
        #endregion



        #region impl
        void StartServer()
        {
            StopServer();

            var settings = ServerSettings.default_value;
            settings.port = (ushort)m_port;
            m_server = msServerStart(ref settings);
            m_handler = OnServerEvent;
        }

        void StopServer()
        {
            msServerStop(m_server);
            m_server = IntPtr.Zero;
        }

        void PollServerEvents()
        {
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
            foreach (var mr in FindObjectsOfType<MeshRenderer>())
            {
                ServeData(mr, edata.flags);
            }
            foreach (var smr in FindObjectsOfType<SkinnedMeshRenderer>())
            {
                ServeData(smr, edata.flags);
            }
            msServerEndServe(m_server);

            //Debug.Log("MeshSyncServer: Get");
        }

        void OnRecvDelete(IntPtr data)
        {
            var edata = default(DeleteData);
            msCopyData(EventType.Delete, ref edata, data);
            var path = S(edata.path);
            var target = FindObjectByPath(null, path);
            if (target != null)
            {
                DestroyImmediate(target.gameObject);
            }

            //Debug.Log("MeshSyncServer: Delete " + path);
        }

        void OnRecvMesh(IntPtr data)
        {
            var edata = default(MeshData);
            msCopyData(EventType.Mesh, ref edata, data);
            var path = S(edata.path);

            Transform target = FindObjectByPath(null, path, true);
            if (target == null) { return; }

            // if object is not visible, just disable and return
            if(!edata.flags.visible)
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

            if (edata.flags.has_transform)
            {
                target.localPosition = edata.transform.position;
                target.localRotation = edata.transform.rotation;
                target.localScale = edata.transform.scale;
            }

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
                    t = FindObjectByPath(null, path + "/Submesh[" + i + "]", true);
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
                            int num_indices = msSubmeshGetNumIndices(ref smd, smi);
                            m_indices = new int[num_indices];
                            msSubmeshCopyIndices(RawPtr(m_indices), ref smd, smi);
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

            //Debug.Log("MeshSyncServer: Mesh " + path);
        }


        Transform FindObjectByPath(Transform parent, string path, bool createIfNotExist = false)
        {
            var names = path.Split('/');
            Transform t = null;
            foreach (var name in names)
            {
                if (name.Length == 0) { continue; }
                t = FindObjectByName(t, name, createIfNotExist);
                if (t == null) { break; }
            }
            return t;
        }

        Transform FindObjectByName(Transform parent, string name, bool createIfNotExist = false)
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
                mfilter.sharedMesh = mesh;
                mesh.MarkDynamic();

                var renderer = go.AddComponent<MeshRenderer>();
                if (go.name.StartsWith("Submesh"))
                {
                    var parent = go.GetComponent<Transform>().parent.GetComponent<Renderer>();
                    renderer.sharedMaterials = parent.sharedMaterials;
                }
                else
                {
#if UNITY_EDITOR
                    Material[] materials = new Material[] { UnityEngine.Object.Instantiate(GetDefaultMaterial()) };
                    materials[0].name = "Material_0";
                    renderer.sharedMaterials = materials;
#endif
                }
            }
            return mfilter.sharedMesh;
        }

        void ForceRepaint()
        {
#if UNITY_EDITOR
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

        void ServeData(MeshRenderer mr, GetFlags flags)
        {
            var mesh = mr.GetComponent<MeshFilter>().sharedMesh;
            if (mesh == null) return;
            if (!mesh.isReadable)
            {
                Debug.LogWarning("Mesh " + mr.name + " is not readable and be ignored");
                return;
            }

            var data = default(MeshData);
            data.id_unity = mr.gameObject.GetInstanceID();
            data.flags.visible = true;

            if (flags.get_transform)
            {
                data.flags.has_transform = true;
                Capture(ref data.transform, mr.GetComponent<Transform>());
            }
            Capture(ref data, mesh, null, flags);

            data.path = msCreateString(GetPath(mr.GetComponent<Transform>()));
            msServerAddServeData(m_server, EventType.Mesh, ref data);
            msDeleteString(data.path);
        }

        void ServeData(SkinnedMeshRenderer smr, GetFlags flags)
        {
            var data = default(MeshData);
            data.id_unity = smr.gameObject.GetInstanceID();
            data.flags.visible = true;

            if (flags.get_transform)
            {
                data.flags.has_transform = true;
                Capture(ref data.transform, smr.GetComponent<Transform>());
            }
            if (flags.bake_skin)
            {
                Cloth cloth = smr.GetComponent<Cloth>();
                if (cloth != null)
                {
                    var mesh = smr.sharedMesh;
                    if (mesh == null)
                    {
                        return;
                    }
                    if (!mesh.isReadable)
                    {
                        Debug.LogWarning("Mesh " + smr.name + " is not readable and be ignored");
                        return;
                    }
                    Capture(ref data, mesh, cloth, flags);
                }
                else
                {
                    var mesh = new Mesh();
                    smr.BakeMesh(mesh);
                    Capture(ref data, mesh, null, flags);
                }
            }
            else
            {
                var mesh = smr.sharedMesh;
                if (mesh == null) return;
                if (!mesh.isReadable)
                {
                    Debug.LogWarning("Mesh " + smr.name + " is not readable and be ignored");
                    return;
                }
                Capture(ref data, mesh, null, flags);
            }

            data.path = msCreateString(GetPath(smr.GetComponent<Transform>()));
            msServerAddServeData(m_server, EventType.Mesh, ref data);
            msDeleteString(data.path);
        }

        void Capture(ref TransformData data, Transform trans)
        {
            data.position = trans.localPosition;
            data.rotation = trans.localRotation;
            data.rotation_eularZXY = trans.localEulerAngles;
            data.position = trans.localPosition;
            data.local2world = trans.localToWorldMatrix;
            data.world2local = trans.worldToLocalMatrix;
        }

        void Capture(ref MeshData data, Mesh mesh, Cloth cloth, GetFlags flags)
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
            StartServer();
        }
#endif

        void Awake()
        {
            StartServer();
#if UNITY_EDITOR
            EditorApplication.update += PollServerEvents;
#endif
        }

        void OnDestroy()
        {
#if UNITY_EDITOR
            EditorApplication.update -= PollServerEvents;
#endif
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
