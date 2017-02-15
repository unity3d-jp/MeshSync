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
    public partial class MeshSyncServer : MonoBehaviour, ISerializationCallbackReceiver
    {
        #region fields
        [SerializeField] int m_serverPort = 8080;
        [HideInInspector][SerializeField] List<MaterialHolder> m_materialList = new List<MaterialHolder>();
        [SerializeField] string m_assetExportPath = "Assets/MeshSyncAssets";
        [SerializeField] bool m_logging = true;

        IntPtr m_server;
        msMessageHandler m_handler;
        bool m_requestRestartServer = false;
        bool m_captureScreenshotInProgress = false;

        Dictionary<string, Record> m_clientMeshes = new Dictionary<string, Record>();
        Dictionary<int, Record> m_hostMeshes = new Dictionary<int, Record>();
        Dictionary<GameObject, int> m_objIDTable = new Dictionary<GameObject, int>();

        [HideInInspector][SerializeField] string[] m_clientMeshes_keys;
        [HideInInspector][SerializeField] Record[] m_clientMeshes_values;
        [HideInInspector][SerializeField] int[] m_hostMeshes_keys;
        [HideInInspector][SerializeField] Record[] m_hostMeshes_values;
        [HideInInspector][SerializeField] GameObject[] m_objIDTable_keys;
        [HideInInspector][SerializeField] int[] m_objIDTable_values;
        #endregion

        #region properties
        public List<MaterialHolder> materialData { get { return m_materialList; } }
        #endregion

        #region impl
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

        void SerializeDictionary<K,V>(Dictionary<K,V> dic, ref K[] keys, ref V[] values)
        {
            keys = dic.Keys.ToArray();
            values = dic.Values.ToArray();
        }
        void DeserializeDictionary<K, V>(Dictionary<K, V> dic, ref K[] keys, ref V[] values)
        {
            try
            {
                if (keys != null && values != null && keys.Length == values.Length)
                {
                    int n = keys.Length;
                    for (int i = 0; i < n; ++i)
                    {
                        dic[keys[i]] = values[i];
                    }
                }
            }
            catch (Exception)
            {
            }
            keys = null;
            values = null;
        }

        public void OnBeforeSerialize()
        {
            SerializeDictionary(m_clientMeshes, ref m_clientMeshes_keys, ref m_clientMeshes_values);
            SerializeDictionary(m_hostMeshes, ref m_hostMeshes_keys, ref m_hostMeshes_values);
            SerializeDictionary(m_objIDTable, ref m_objIDTable_keys, ref m_objIDTable_values);
        }
        public void OnAfterDeserialize()
        {
            DeserializeDictionary(m_clientMeshes, ref m_clientMeshes_keys, ref m_clientMeshes_values);
            DeserializeDictionary(m_hostMeshes, ref m_hostMeshes_keys, ref m_hostMeshes_values);
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
            SceneView.onSceneGUIDelegate += OnSceneViewGUI;
#endif
        }

        void StopServer()
        {
            if(m_server != IntPtr.Zero)
            {
#if UNITY_EDITOR
                EditorApplication.update -= PollServerEvents;
                SceneView.onSceneGUIDelegate -= OnSceneViewGUI;
#endif
                msServerStop(m_server);
                m_server = IntPtr.Zero;
            }
        }

        void PollServerEvents()
        {
            if(m_requestRestartServer)
            {
                m_requestRestartServer = false;
                StartServer();
            }
            if (m_captureScreenshotInProgress)
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
                case MessageType.Fence:
                    OnRecvFence((FenceMessage)data);
                    break;
                case MessageType.Text:
                    OnRecvText((TextMessage)data);
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

        void OnRecvFence(FenceMessage mes)
        {
            if(mes.type == FenceMessage.FenceType.SceneEnd)
            {
                SortObjects();
            }
        }

        void OnRecvText(TextMessage mes)
        {
            mes.Print();
        }

        void OnRecvSet(SetMessage mes)
        {
#if UNITY_EDITOR
            Undo.RecordObject(this, "MeshSyncServer");
#endif
            var scene = mes.scene;
            // sync materials
            int numMaterials = scene.numMaterials;
            if(numMaterials > 0)
            {
                UpdateMaterials(scene);
            }

            // sync transforms
            int numTransforms = scene.numTransforms;
            for (int i = 0; i < numTransforms; ++i)
            {
                UpdateTransform(scene.GetTransform(i));
            }

            // sync cameras
            int numCameras = scene.numCameras;
            for (int i = 0; i < numCameras; ++i)
            {
                UpdateCamera(scene.GetCamera(i));
            }

            // sync meshes
            int numMeshes = scene.numMeshes;
            for (int i = 0; i < numMeshes; ++i)
            {
                UpdateMesh(scene.GetMesh(i));
            }

            //Debug.Log("MeshSyncServer: Set");
        }

        void UpdateMaterials(SceneData scene)
        {
            int numMaterials = scene.numMaterials;

            bool needsUpdate = false;
            if(m_materialList.Count != numMaterials)
            {
                needsUpdate = true;
            }
            else
            {
                for (int i = 0; i < numMaterials; ++i)
                {
                    var src = scene.GetMaterial(i);
                    var dst = m_materialList[i];
                    if(src.id != dst.id || src.name != dst.name || src.color != dst.color)
                    {
                        needsUpdate = true;
                        break;
                    }
                }
            }
            if(!needsUpdate) { return; }

            var newlist = new List<MaterialHolder>();
            for (int i = 0; i < numMaterials; ++i)
            {
                var src = scene.GetMaterial(i);
                var id = src.id;
                var dst = m_materialList.Find(a => a.id == id);
                if (dst == null)
                {
                    dst = new MaterialHolder();
                    dst.id = id;
#if UNITY_EDITOR
                    var tmp = Instantiate(GetDefaultMaterial());
                    tmp.name = src.name;
                    tmp.color = src.color;
                    dst.material = tmp;
#endif
                }
                dst.name = src.name;
                dst.color = src.color;
                if (dst.material.name == src.name)
                {
                    dst.material.color = dst.color;
                }
                newlist.Add(dst);
            }
            m_materialList = newlist;
            ReassignMaterials();
        }

        void UpdateMesh(MeshData data)
        {
            var data_trans = data.transform;
            var data_id = data_trans.id;
            var path = data_trans.path;
            bool createdNewMesh = false;

            // find or create target object
            Record rec = null;
            if(data_id != 0 && m_hostMeshes.ContainsKey(data_id))
            {
                rec = m_hostMeshes[data_id];
                if(rec.go == null)
                {
                    m_hostMeshes.Remove(data_id);
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
            rec.index = data_trans.index;


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
            var flags = data.flags;

            // update transform
            if(data.flags.applyTRS) {
                var trs = data_trans.trs;
                target.localPosition = trs.position;
                target.localRotation = trs.rotation;
                target.localScale = trs.scale;
            }

            bool skinned = data.numBones > 0;

            var smr = rec.go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null && !rec.recved && !skinned)
            {
                // update skinned mesh - only when topology is not changed
                rec.editMesh = CreateEditedMesh(data, data.GetSplit(0), true, rec.origMesh);
                if (rec.editMesh == null)
                {
                    if (m_logging)
                    {
                        Debug.Log("edit for " + rec.origMesh.name + " is ignored. currently changing topology of skinned meshes is not supported.");
                    }
                }
                else
                {
                    smr.sharedMesh = rec.editMesh;
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

                    var split = data.GetSplit(i);
                    rec.editMesh = CreateEditedMesh(data, split);
                    rec.editMesh.name = i == 0 ? target.name : target.name + "[" + i + "]";

                    if (skinned)
                    {
                        smr = GetOrAddSkinnedMeshComponents(t.gameObject, i > 0);
                        if(smr != null)
                        {
                            smr.sharedMesh = rec.editMesh;

                            var bones = GetOrCreateBones(data);
                            if(bones.Length > 0)
                            {
                                smr.bones = bones;
                                smr.rootBone = bones[0];
                            }
                        }
                    }
                    else
                    {
                        var mfilter = GetOrAddMeshComponents(t.gameObject, i > 0);
                        if (mfilter != null)
                        {
                            mfilter.sharedMesh = rec.editMesh;
                        }
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

            if (skinned)
            {
                rec.go.SetActive(false); // 
                rec.go.SetActive(true);  // force recalculate skinned mesh in editor. I couldn't find better way...
            }

            // assign materials if needed
            if (materialsUpdated)
            {
                AssignMaterials(rec);
            }
        }

        Transform[] GetOrCreateBones(MeshData data)
        {
            var paths = data.GetBonePaths();
            var ret = new Transform[data.numBones];
            for (int i = 0; i < ret.Length; ++i)
            {
                ret[i] = FindObjectByPath(null, paths[i]);
            }
            return ret;
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
            if (flags.hasBones)
            {
                mesh.boneWeights = split.boneWeights;
                mesh.bindposes = data.bindposes;
            }

            if(!noTopologyUpdate && flags.hasIndices)
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

        void SortObjects()
        {
            var rec = m_clientMeshes.Values.OrderBy(v => v.index);
            foreach(var r in rec)
            {
                if(r.go != null)
                {
                    r.go.GetComponent<Transform>().SetSiblingIndex(r.index + 1000);
                }
            }
        }

        Transform UpdateTransform(TransformData data)
        {
            bool created = false;
            var trans = FindObjectByPath(null, data.path, true, ref created);

            var trs = data.trs;
            trans.localPosition = trs.position;
            trans.localRotation = trs.rotation;
            trans.localScale = trs.scale;
            return trans;
        }

        Camera UpdateCamera(CameraData data)
        {
            var trans = UpdateTransform(data.transform);
            var cam = trans.GetComponent<Camera>();
            if(cam == null)
            {
                cam = trans.gameObject.AddComponent<Camera>();
            }
            cam.fieldOfView = data.fov;
            return cam;
        }

        public void ReassignMaterials()
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
                if( r== null) { continue; }

                int submeshCount = submeshCounts[i];
                var prev = r.sharedMaterials;
                var materials = new Material[submeshCount];
                bool changed = false;

                for (int j = 0; j < submeshCount; ++j)
                {
                    if (j < prev.Length && prev[j] != null)
                    {
                        materials[j] = prev[j];
                    }

                    var mid = materialIDs[mi++];
                    if(mid >= 0 && mid < m_materialList.Count)
                    {
                        if(materials[j] != m_materialList[mid].material)
                        {
                            materials[j] = m_materialList[mid].material;
                            changed = true;
                        }
                    }
                    else
                    {
                        if(materials[j] == null) {
#if UNITY_EDITOR
                            var tmp = Instantiate(GetDefaultMaterial());
                            tmp.name = "DefaultMaterial";
                            materials[j] = tmp;
                            changed = true;
#endif
                        }
                    }
                }

                if(changed)
                {
#if UNITY_EDITOR
                    //Undo.RecordObjects( new UnityEngine.Object[] { this, r }, "Update Materials");
#endif
                    r.sharedMaterials = materials;
                }
            }
        }

        int GetMaterialIndex(Material mat)
        {
            if(mat == null) { return -1; }

            for (int i = 0; i < m_materialList.Count; ++i)
            {
                if(m_materialList[i].material == mat)
                {
                    return i;
                }
            }

            int ret = m_materialList.Count;
            var tmp = new MaterialHolder();
            tmp.name = mat.name;
            tmp.material = mat;
            tmp.id = ret + 1;
            m_materialList.Add(tmp);
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

        SkinnedMeshRenderer GetOrAddSkinnedMeshComponents(GameObject go, bool isSplit)
        {
            var smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr == null)
            {
                smr = go.AddComponent<SkinnedMeshRenderer>();
                {
                    // destroy mesh components if exist
                    var mfilter = go.GetComponent<MeshFilter>();
                    var mr = go.AddComponent<MeshRenderer>();
                    if (mfilter != null && mr != null)
                    {
                        smr.materials = mr.materials;
                        DestroyImmediate(mfilter);
                        DestroyImmediate(mr);
                    }
                }
                if (isSplit)
                {
                    var parent = go.GetComponent<Transform>().parent.GetComponent<Renderer>();
                    smr.sharedMaterials = parent.sharedMaterials;
                }
            }

            return smr;
        }

        public void ForceRepaint()
        {
#if UNITY_EDITOR
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
                var dst_trans = dst.transform;
                dst_trans.id = GetObjectlID(renderer.gameObject);
                var trans = renderer.GetComponent<Transform>();
                if (mes.flags.getTransform)
                {
                    var trs = default(TRS);
                    trs.position = trans.localPosition;
                    trs.rotation = trans.localRotation;
                    trs.rotation_eularZXY = trans.localEulerAngles;
                    trs.scale = trans.localScale;
                    dst_trans.trs = trs;
                }
                dst.local2world = trans.localToWorldMatrix;
                dst.world2local = trans.worldToLocalMatrix;

                if (!m_hostMeshes.ContainsKey(dst_trans.id))
                {
                    m_hostMeshes[dst_trans.id] = new Record();
                }
                var rec = m_hostMeshes[dst_trans.id];
                rec.go = renderer.gameObject;
                rec.origMesh = origMesh;

                dst_trans.path = BuildPath(renderer.GetComponent<Transform>());
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

                if (mes.flags.getBones)
                {
                    dst.SetBonePaths(smr.bones);
                    dst.bindposes = mesh.bindposes;
                    dst.boneWeights = mesh.boneWeights;
                }
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
                        int mid = i < materials.Length ? GetMaterialIndex(materials[i]) : 0;
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

        void CheckMaterialAssignedViaEditor()
        {
            bool changed = false;
            foreach(var kvp in m_clientMeshes)
            {
                var rec = kvp.Value;
                if (rec.go != null && rec.go.activeInHierarchy)
                {
                    var mr = rec.go.GetComponent<MeshRenderer>();
                    if(mr == null || rec.submeshCounts.Length == 0) { continue; }

                    var materials = mr.sharedMaterials;
                    int n = Math.Min(materials.Length, rec.submeshCounts[0]);
                    for (int i = 0; i < n; ++i)
                    {
                        int midx = rec.materialIDs[i];
                        if(midx < 0 || midx >= m_materialList.Count) { continue; }

                        if(materials[i] != m_materialList[midx].material)
                        {
                            m_materialList[midx].material = materials[i];
                            changed = true;
                            break;
                        }
                    }
                }
                if(changed) { break; }
            }

            if(changed)
            {
                ReassignMaterials();
                ForceRepaint();
            }
        }

        void OnSceneViewGUI(SceneView sceneView)
        {
            if (Event.current.type == EventType.DragExited)
            {
                if (Event.current.button == 0)
                {
                    CheckMaterialAssignedViaEditor();
                }
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
            m_requestRestartServer = true;
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
