using System;
using System.Linq;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEngine.Animations;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.MeshSync
{
    [ExecuteInEditMode]
    public partial class MeshSyncServer : MonoBehaviour, ISerializationCallbackReceiver
    {
        #region fields
        [SerializeField] int m_serverPort = 8080;
        [HideInInspector][SerializeField] List<MaterialHolder> m_materialList = new List<MaterialHolder>();
        [SerializeField] string m_assetExportPath = "MeshSyncAssets";
        [SerializeField] bool m_logging = true;

        IntPtr m_server;
        msMessageHandler m_handler;
        bool m_requestRestartServer = false;
        bool m_captureScreenshotInProgress = false;

        Dictionary<string, Record> m_clientObjects = new Dictionary<string, Record>();
        Dictionary<int, Record> m_hostObjects = new Dictionary<int, Record>();
        Dictionary<GameObject, int> m_objIDTable = new Dictionary<GameObject, int>();

        [HideInInspector][SerializeField] string[] m_clientObjects_keys;
        [HideInInspector][SerializeField] Record[] m_clientObjects_values;
        [HideInInspector][SerializeField] int[] m_hostObjects_keys;
        [HideInInspector][SerializeField] Record[] m_hostObjects_values;
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
            SerializeDictionary(m_clientObjects, ref m_clientObjects_keys, ref m_clientObjects_values);
            SerializeDictionary(m_hostObjects, ref m_hostObjects_keys, ref m_hostObjects_values);
            SerializeDictionary(m_objIDTable, ref m_objIDTable_keys, ref m_objIDTable_values);
        }
        public void OnAfterDeserialize()
        {
            DeserializeDictionary(m_clientObjects, ref m_clientObjects_keys, ref m_clientObjects_values);
            DeserializeDictionary(m_hostObjects, ref m_hostObjects_keys, ref m_hostObjects_values);
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

            if (msServerGetNumMessages(m_server) > 0)
            {
                msServerProcessMessages(m_server, m_handler);
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
                ServeMesh(mr, mes);
            }
            foreach(var mat in m_materialList)
            {
                ServeMaterial(mat.material, mes);
            }
            msServerEndServe(m_server);

            //Debug.Log("MeshSyncServer: Get");
        }

        void OnRecvDelete(DeleteMessage mes)
        {
            int numTargets = mes.numTargets;
            for (int i = 0; i < numTargets; ++i)
            {
                var id = mes.GetID(i);
                var path = mes.GetPath(i);

                if (id != 0 && m_hostObjects.ContainsKey(id))
                {
                    var rec = m_hostObjects[id];
                    if (rec.go != null)
                    {
                        DestroyImmediate(rec.go);
                    }
                    m_hostObjects.Remove(id);
                }
                else if (m_clientObjects.ContainsKey(path))
                {
                    var rec = m_clientObjects[path];
                    if (rec.go != null)
                    {
                        DestroyImmediate(rec.go);
                    }
                    m_clientObjects.Remove(path);
                }
            }

            //Debug.Log("MeshSyncServer: Delete");
        }

        void OnRecvFence(FenceMessage mes)
        {
            if(mes.type == FenceMessage.FenceType.SceneEnd)
            {
                // sort objects by index
                {
                    var rec = m_clientObjects.Values.OrderBy(v => v.index);
                    foreach (var r in rec)
                    {
                        if (r.go != null)
                        {
                            r.go.GetComponent<Transform>().SetSiblingIndex(r.index + 1000);
                        }
                    }
                }

                // force recalculate skinning
                foreach (var rec in m_clientObjects)
                {
                    if(rec.Value.editMesh)
                    {
                        var go = rec.Value.go;
                        if(go.activeInHierarchy)
                        {
                            go.SetActive(false); // 
                            go.SetActive(true);  // force recalculate skinned mesh on editor. I couldn't find better way...
                        }
                    }
                }

                ForceRepaint();
                GC.Collect();
            }
        }

        void OnRecvText(TextMessage mes)
        {
            mes.Print();
        }

        void OnRecvSet(SetMessage mes)
        {
            var scene = mes.scene;

            // sync materials
            try
            {
                int numMaterials = scene.numMaterials;
                if (numMaterials > 0)
                    UpdateMaterials(scene);
            }
            catch (Exception e) { Debug.Log(e); }

            // sync transforms
            try
            {
                int numObjects = scene.numObjects;
                for (int i = 0; i < numObjects; ++i)
                {
                    var obj = scene.GetObject(i);
                    switch(obj.type)
                    {
                        case TransformData.Type.Transform:
                            UpdateTransform(obj);
                            break;
                        case TransformData.Type.Camera:
                            UpdateCamera((CameraData)obj._this);
                            break;
                        case TransformData.Type.Light:
                            UpdateLight((LightData)obj._this);
                            break;
                        case TransformData.Type.Mesh:
                            UpdateMesh((MeshData)obj._this);
                            break;
                    }
                }
            }
            catch (Exception e) { Debug.Log(e); }

#if UNITY_2018_1_OR_NEWER
            try
            {
                int numConstraints = scene.numConstraints;
                for (int i = 0; i < numConstraints; ++i)
                    UpdateConstraint(scene.GetConstraint(i));
            }
            catch (Exception e) { Debug.Log(e); }
#endif
            try
            {
                int numAnimations = scene.numAnimations;
                for (int i = 0; i < numAnimations; ++i)
                    UpdateAnimation(scene.GetAnimation(i));
            }
            catch (Exception e) { Debug.Log(e); }

            // update references
            {
                foreach (var pair in m_clientObjects)
                {
                    var dstrec = pair.Value;
                    if (dstrec.go == null)
                        continue;
                    else if (dstrec.reference == null)
                        continue;

                    Record srcrec = null;
                    if(m_clientObjects.TryGetValue(dstrec.reference, out srcrec))
                    {
                        if (srcrec.go == null)
                            continue;
                        var src = srcrec.go.GetComponent<SkinnedMeshRenderer>();
                        if (src == null)
                            continue;
                        var dst = dstrec.go.GetComponent<SkinnedMeshRenderer>();
                        if (dst == null)
                            dst = dstrec.go.AddComponent<SkinnedMeshRenderer>();
                        dst.sharedMesh = src.sharedMesh;
                        dst.sharedMaterials = src.sharedMaterials;
                    }
                }
            }
        }

        void DestroyIfNotAsset(UnityEngine.Object obj)
        {
            if(obj != null
#if UNITY_EDITOR
                && AssetDatabase.GetAssetPath(obj) == ""
#endif
                )
            {
                DestroyImmediate(obj, false);
            }
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
            var trans = UpdateTransform(data.transform);
            if (trans == null) { return; }

            var data_trans = data.transform;
            var data_id = data_trans.id;
            var path = data_trans.path;

            Record rec = null;
            if (!m_clientObjects.TryGetValue(path, out rec))
                m_hostObjects.TryGetValue(data_id, out rec);
            if (rec == null)
            {
                Debug.LogError("Something wrong");
                return;
            }

            var target = rec.go.GetComponent<Transform>();
            var go = target.gameObject;

            bool activeInHierarchy = go.activeInHierarchy;
            if (!activeInHierarchy && !data.flags.hasPoints) { return; }


            // allocate material list
            bool materialsUpdated = rec.BuildMaterialData(data);
            var flags = data.flags;
            bool skinned = data.numBones > 0;

            // update mesh
            for (int si = 0; si < data.numSplits; ++si)
            {
                var t = target;
                if (si > 0)
                {
                    bool created = false;
                    t = FindOrCreateObjectByPath(path + "/[" + si + "]", true, ref created);
                    t.gameObject.SetActive(true);
                }

                var split = data.GetSplit(si);
                if(split.numPoints == 0 || split.numIndices == 0)
                {
                    rec.editMesh = null;
                }
                else
                {
                    rec.editMesh = CreateEditedMesh(data, split);
                    rec.editMesh.name = si == 0 ? target.name : target.name + "[" + si + "]";
                }

                var smr = GetOrAddSkinnedMeshRenderer(t.gameObject, si > 0);
                if (smr != null)
                {
                    {
                        var old = smr.sharedMesh;
                        smr.sharedMesh = null;
                        DestroyIfNotAsset(old);
                        old = null;
                    }

                    bool updateWhenOffscreen = false;
                    if (skinned)
                    {
                        // create bones
                        var paths = data.GetBonePaths();
                        var bones = new Transform[data.numBones];
                        for (int bi = 0; bi < bones.Length; ++bi)
                        {
                            bool created = false;
                            bones[bi] = FindOrCreateObjectByPath(paths[bi], true, ref created);
                            if (created)
                            {
                                var newrec = new Record
                                {
                                    go = bones[bi].gameObject,
                                    recved = true,
                                };
                                if (m_clientObjects.ContainsKey(paths[bi]))
                                    m_clientObjects[paths[bi]] = newrec;
                                else
                                    m_clientObjects.Add(paths[bi], newrec);
                            }
                        }

                        if (bones.Length > 0)
                        {
                            bool created = false;
                            var root = FindOrCreateObjectByPath(data.rootBonePath, true, ref created);
                            if (root == null)
                                root = bones[0];
                            smr.rootBone = root;
                            smr.bones = bones;
                            updateWhenOffscreen = true;
                        }
                    }
                    else
                    {
                        if (smr.rootBone != null)
                        {
                            smr.bones = null;
                            smr.rootBone = null;
                        }

                        if (rec.editMesh != null)
                            smr.localBounds = rec.editMesh.bounds;
                    }

                    smr.sharedMesh = rec.editMesh;
                    smr.updateWhenOffscreen = updateWhenOffscreen;

                    if (flags.hasBlendshapes)
                    {
                        int numBlendShapes = data.numBlendShapes;
                        for (int bi = 0; bi < numBlendShapes; ++bi)
                        {
                            var bsd = data.GetBlendShapeData(bi);
                            smr.SetBlendShapeWeight(bi, bsd.weight);
                        }
                    }
                }

                var renderer = trans.gameObject.GetComponent<Renderer>();
                if (renderer != null)
                    renderer.enabled = data.transform.visible;
            }

            int num_splits = Math.Max(1, data.numSplits);
            for (int si = num_splits; ; ++si)
            {
                bool created = false;
                var t = FindOrCreateObjectByPath(path + "/[" + si + "]", false, ref created);
                if (t == null) { break; }
                DestroyImmediate(t.gameObject);
            }

            // assign materials if needed
            if (materialsUpdated)
                AssignMaterials(rec);
        }

        PinnedList<Vector2> tmpV2 = new PinnedList<Vector2>();
        PinnedList<Vector3> tmpV3 = new PinnedList<Vector3>();
        PinnedList<Vector4> tmpV4 = new PinnedList<Vector4>();
        PinnedList<Color> tmpC = new PinnedList<Color>();

        Mesh CreateEditedMesh(MeshData data, SplitData split)
        {
            var mesh = new Mesh();
#if UNITY_2017_3_OR_NEWER
            mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
#endif

            var flags = data.flags;
            if (flags.hasPoints)
            {
                tmpV3.Resize(split.numPoints);
                data.ReadPoints(tmpV3, split);
                mesh.SetVertices(tmpV3.List);
            }
            if (flags.hasNormals)
            {
                tmpV3.Resize(split.numPoints);
                data.ReadNormals(tmpV3, split);
                mesh.SetNormals(tmpV3.List);
            }
            if (flags.hasTangents)
            {
                tmpV4.Resize(split.numPoints);
                data.ReadTangents(tmpV4, split);
                mesh.SetTangents(tmpV4.List);
            }
            if (flags.hasUV0)
            {
                tmpV2.Resize(split.numPoints);
                data.ReadUV0(tmpV2, split);
                mesh.SetUVs(0, tmpV2.List);
            }
            if (flags.hasUV1)
            {
                tmpV2.Resize(split.numPoints);
                data.ReadUV1(tmpV2, split);
                mesh.SetUVs(1, tmpV2.List);
            }
            if (flags.hasColors)
            {
                tmpC.Resize(split.numPoints);
                data.ReadColors(tmpC, split);
                mesh.SetColors(tmpC.List);
            }
            if (flags.hasBones)
            {
                var tmpW = new PinnedList<BoneWeight>();
                tmpW.Resize(split.numPoints);
                data.ReadBoneWeights(tmpW, split);
                mesh.bindposes = data.bindposes;
                mesh.boneWeights = tmpW.Array;
            }
            if(flags.hasIndices)
            {
                mesh.subMeshCount = split.numSubmeshes;
                for (int smi = 0; smi < mesh.subMeshCount; ++smi)
                {
                    var submesh = split.GetSubmesh(smi);
                    var topology = submesh.topology;

                    if (topology == SubmeshData.Topology.Triangles)
                        mesh.SetTriangles(submesh.indices, smi, false);
                    else if (topology == SubmeshData.Topology.Lines)
                        mesh.SetIndices(submesh.indices, MeshTopology.Lines, smi, false);
                    else if (topology == SubmeshData.Topology.Points)
                        mesh.SetIndices(submesh.indices, MeshTopology.Points, smi, false);
                    else if (topology == SubmeshData.Topology.Quads)
                        mesh.SetIndices(submesh.indices, MeshTopology.Quads, smi, false);

                }
            }
            if (flags.hasBlendshapes)
            {
                PinnedList<Vector3> tmpBSP = new PinnedList<Vector3>(split.numPoints);
                PinnedList<Vector3> tmpBSN = new PinnedList<Vector3>(split.numPoints);
                PinnedList<Vector3> tmpBST = new PinnedList<Vector3>(split.numPoints);

                int numBlendShapes = data.numBlendShapes;
                for (int bi = 0; bi < numBlendShapes; ++bi)
                {
                    var bsd = data.GetBlendShapeData(bi);
                    var name = bsd.name;
                    var numFrames = bsd.numFrames;
                    for (int fi = 0; fi < numFrames; ++fi)
                    {
                        bsd.ReadPoints(fi, tmpBSP.Array, split);
                        bsd.ReadNormals(fi, tmpBSN.Array, split);
                        bsd.ReadTangents(fi, tmpBST.Array, split);
                        mesh.AddBlendShapeFrame(name, bsd.GetWeight(fi), tmpBSP.Array, tmpBSN.Array, tmpBST.Array);
                    }
                }
            }

            mesh.bounds = split.bounds;
            mesh.UploadMeshData(false);
            return mesh;
        }

        void CreateAsset(UnityEngine.Object obj, string path)
        {
#if UNITY_EDITOR
            string assetDir = "Assets/" + m_assetExportPath;
            if (!AssetDatabase.IsValidFolder(assetDir))
                AssetDatabase.CreateFolder("Assets", m_assetExportPath);
            AssetDatabase.CreateAsset(obj, path);
#endif
        }

        Transform UpdateTransform(TransformData data)
        {
            var path = data.path;
            int data_id = data.id;
            if(path.Length == 0) { return null; }

            Transform trans = null;
            Record rec = null;
            if (data_id != 0)
            {
                if (m_hostObjects.TryGetValue(data_id, out rec))
                {
                    if (rec.go == null)
                    {
                        m_hostObjects.Remove(data_id);
                        rec = null;
                    }
                }
            }
            else
            {
                if (m_clientObjects.TryGetValue(path, out rec))
                {
                    if (rec.go == null)
                    {
                        m_clientObjects.Remove(path);
                        rec = null;
                    }
                }
            }

            if (rec != null)
            {
                trans = rec.go.GetComponent<Transform>();
            }
            else
            {
                bool created = false;
                trans = FindOrCreateObjectByPath(path, true, ref created);
                rec = new Record
                {
                    go = trans.gameObject,
                    recved = true,
                };
                m_clientObjects.Add(path, rec);
            }

            rec.index = data.index;
            var reference = data.reference;
            rec.reference = reference != "" ? reference : null;

            // import TRS
            trans.localPosition = data.position;
            trans.localRotation = data.rotation;
            trans.localScale = data.scale;

            // visibility
            trans.gameObject.SetActive(data.visibleHierarchy);

            return trans;
        }

        Camera UpdateCamera(CameraData data)
        {
            var trans = UpdateTransform(data.transform);
            if (trans == null) { return null; }

            var cam = trans.GetComponent<Camera>();
            if(cam == null)
                cam = trans.gameObject.AddComponent<Camera>();
            cam.orthographic = data.orthographic;
            cam.fieldOfView = data.fov;
            cam.nearClipPlane = data.nearClipPlane;
            cam.farClipPlane = data.farClipPlane;
            cam.enabled = data.transform.visible;
            return cam;
        }

        Light UpdateLight(LightData data)
        {
            var trans = UpdateTransform(data.transform);
            if (trans == null) { return null; }

            var lt = trans.GetComponent<Light>();
            if (lt == null)
            {
                lt = trans.gameObject.AddComponent<Light>();
            }

            lt.type = data.type;
            lt.color = data.color;
            lt.intensity = data.intensity;
            if (data.range > 0.0f) { lt.range = data.range; }
            if (data.spotAngle > 0.0f) { lt.spotAngle = data.spotAngle; }
            lt.enabled = data.transform.visible;
            return lt;
        }

        T GetOrAddComponent<T>(GameObject go) where T : Component
        {
            var ret = go.GetComponent<T>();
            if (ret == null)
                ret = go.AddComponent<T>();
            return ret;
        }

        void UpdateConstraint(ConstraintData data)
        {
#if UNITY_2018_1_OR_NEWER
            bool dummy = false;
            var trans = FindOrCreateObjectByPath(data.path, false, ref dummy);
            if (trans == null)
                return;

            Action<IConstraint> basicSetup = (c) =>
            {
                int ns = data.numSources;
                while(c.sourceCount < ns)
                    c.AddSource(new ConstraintSource());
                for (int si = 0; si < ns; ++si)
                {
                    var s = c.GetSource(si);
                    s.sourceTransform = FindOrCreateObjectByPath(data.GetSourcePath(si), false, ref dummy);
                }
            };

            switch (data.type)
            {
                case ConstraintData.ConstraintType.Aim:
                    {
                        var c = GetOrAddComponent<AimConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Parent:
                    {
                        var c = GetOrAddComponent<ParentConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Position:
                    {
                        var c = GetOrAddComponent<PositionConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Rotation:
                    {
                        var c = GetOrAddComponent<RotationConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Scale:
                    {
                        var c = GetOrAddComponent<ScaleConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                default:
                    break;
            }
#endif
        }

        void UpdateAnimation(AnimationData data)
        {
#if UNITY_EDITOR
            var path = data.path;
            bool dummy = false;
            var trans = FindOrCreateObjectByPath(path, false, ref dummy);
            if (trans == null)
                return;

            var tdata = data.transform;
            if (tdata)
            {
                // import TRS animation

                Transform root = trans;
                while (root.parent != null)
                    root = root.parent;

                Animator animator = null;
                AnimationClip clip = null;

                animator = root.GetComponent<Animator>();
                if (animator == null)
                {
                    animator = root.gameObject.AddComponent<Animator>();
                }
                else
                {
                    if (animator.runtimeAnimatorController != null)
                    {
                        var clips = animator.runtimeAnimatorController.animationClips;
                        if (clips != null && clips.Length > 0)
                        {
                            clip = animator.runtimeAnimatorController.animationClips[0];
                        }
                    }
                }

                if (clip == null)
                {
                    clip = new AnimationClip();
                    var assetPath = "Assets/" + m_assetExportPath + "/" + SanitizeFileName(root.name);
                    CreateAsset(clip, assetPath + ".anim");
                    animator.runtimeAnimatorController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPathWithClip(assetPath + ".controller", clip);
                }

                var animPath = path.Replace("/" + root.name, "");
                if (animPath.Length > 0 && animPath[0] == '/')
                {
                    animPath = animPath.Remove(0, 1);
                }
                tdata.ExportToClip(clip, animPath, false);
            }
#endif
        }

        public void ReassignMaterials()
        {
            foreach (var rec in m_clientObjects)
            {
                AssignMaterials(rec.Value);
            }
            foreach (var rec in m_hostObjects)
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
                    var t = rec.go.transform.Find("[" + i + "]");
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

        Transform FindOrCreateObjectByPath(string path, bool createIfNotExist, ref bool created)
        {
            var names = path.Split('/');
            Transform t = null;
            foreach (var name in names)
            {
                if (name.Length == 0) { continue; }
                t = FindOrCreateObjectByName(t, name, createIfNotExist, ref created);
                if (t == null) { break; }
            }
            return t;
        }

        Transform FindOrCreateObjectByName(Transform parent, string name, bool createIfNotExist, ref bool created)
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
                ret = parent.Find(name);
            }

            if (createIfNotExist && ret == null)
            {
                var go = new GameObject();
                go.name = name;
                ret = go.GetComponent<Transform>();
                if (parent != null)
                    ret.SetParent(parent, false);
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

        SkinnedMeshRenderer GetOrAddSkinnedMeshRenderer(GameObject go, bool isSplit)
        {
            var smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr == null)
            {
                smr = go.AddComponent<SkinnedMeshRenderer>();
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

        bool ServeMesh(Renderer renderer, GetMessage mes)
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
                var trans = renderer.GetComponent<Transform>();
                dst_trans.id = GetObjectlID(renderer.gameObject);
                dst_trans.position = trans.localPosition;
                dst_trans.rotation = trans.localRotation;
                dst_trans.scale = trans.localScale;
                dst.local2world = trans.localToWorldMatrix;
                dst.world2local = trans.worldToLocalMatrix;

                if (!m_hostObjects.ContainsKey(dst_trans.id))
                {
                    m_hostObjects[dst_trans.id] = new Record();
                }
                var rec = m_hostObjects[dst_trans.id];
                rec.go = renderer.gameObject;
                rec.origMesh = origMesh;

                dst_trans.path = BuildPath(renderer.GetComponent<Transform>());
                msServerServeMesh(m_server, dst);
            }
            return ret;
        }
        bool ServeMaterial(Material mat, GetMessage mes)
        {
            var data = MaterialData.Create();
            data.name = mat.name;
            data.color = mat.HasProperty("_Color") ? mat.color : Color.white;
            msServerServeMaterial(m_server, data);
            return true;
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
                    dst.WriteWeights(mesh.boneWeights);
                }
            }
            return true;
        }

        void CaptureMesh(ref MeshData data, Mesh mesh, Cloth cloth, GetFlags flags, Material[] materials)
        {
            // todo: cloth?
            if (flags.getPoints)
            {
                data.WritePoints(mesh.vertices);
            }
            if (flags.getNormals)
            {
                data.WriteNormals(mesh.normals);
            }
            if (flags.getTangents)
            {
                data.WriteTangents(mesh.tangents);
            }
            if (flags.getUV0)
            {
                data.WriteUV0(mesh.uv);
            }
            if (flags.getUV1)
            {
                data.WriteUV1(mesh.uv2);
            }
            if (flags.getColors)
            {
                data.WriteColors(mesh.colors);
            }
            if (flags.getIndices)
            {
                if(!flags.getMaterialIDs || materials == null || materials.Length == 0)
                {
                    data.WriteIndices(mesh.triangles);
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
                data.WriteWeights(mesh.boneWeights);
                data.bindposes = mesh.bindposes;
            }
            if (flags.getBlendShapes && mesh.blendShapeCount > 0)
            {
                var v = new Vector3[mesh.vertexCount];
                var n = new Vector3[mesh.vertexCount];
                var t = new Vector3[mesh.vertexCount];
                for (int bi = 0; bi < mesh.blendShapeCount; ++bi)
                {
                    var bd = data.AddBlendShape(mesh.GetBlendShapeName(bi));
                    int frameCount = mesh.GetBlendShapeFrameCount(bi);
                    for (int fi = 0; fi < frameCount; ++fi)
                    {
                        mesh.GetBlendShapeFrameVertices(bi, fi, v, n, t);
                        float w = mesh.GetBlendShapeFrameWeight(bi, fi);
                        bd.AddFrame(w, v, n, t);
                    }
                }
            }
        }

        void OnRecvScreenshot(IntPtr data)
        {
            ScreenCapture.CaptureScreenshot("screenshot.png");
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
                var t = go.transform.Find("[" + i + "]");
                if (t == null) { break; }
                GenerateLightmapUV(t.gameObject);
            }
        }
        public void GenerateLightmapUV()
        {
            foreach (var kvp in m_clientObjects)
            {
                GenerateLightmapUV(kvp.Value.go);
            }
            foreach (var kvp in m_hostObjects)
            {
                if(kvp.Value.editMesh != null)
                    GenerateLightmapUV(kvp.Value.go);
            }
        }

        public void ExportMeshes(GameObject go)
        {
            if(go == null) { return; }

            if (!AssetDatabase.IsValidFolder("Assets/" + m_assetExportPath))
                AssetDatabase.CreateFolder("Assets", m_assetExportPath);
            var mf = go.GetComponent<SkinnedMeshRenderer>();
            if (mf != null && mf.sharedMesh != null)
            {
                var path = "Assets/" + m_assetExportPath + "/" + SanitizeFileName(mf.sharedMesh.name) + ".asset";
                CreateAsset(mf.sharedMesh, path);
                if (m_logging)
                {
                    Debug.Log("exported mesh " + path);
                }

                for (int i=1; ; ++i)
                {
                    var t = go.transform.Find("[" + i + "]");
                    if(t == null) { break; }
                    ExportMeshes(t.gameObject);
                }
            }
        }
        public void ExportMeshes()
        {
            // export client meshes
            foreach (var kvp in m_clientObjects)
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
            foreach (var kvp in m_hostObjects)
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
            foreach(var kvp in m_clientObjects)
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
