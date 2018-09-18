using System;
using System.Linq;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
#if UNITY_2017_1_OR_NEWER
using UnityEngine.Animations;
#endif
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
        [HideInInspector] [SerializeField] List<MaterialHolder> m_materialList = new List<MaterialHolder>();
        [HideInInspector] [SerializeField] List<TextureHolder> m_textureList = new List<TextureHolder>();
        [SerializeField] string m_assetExportPath = "MeshSyncAssets";
        [SerializeField] Transform m_rootObject;
        [Space(10)]
        [SerializeField] bool m_syncTransform = true;
        [SerializeField] bool m_syncVisibility = true;
        [SerializeField] bool m_syncCameras = true;
        [SerializeField] bool m_syncLights = true;
        [SerializeField] bool m_syncMeshes = true;
        [SerializeField] bool m_updateMeshColliders = true;
        [Space(10)]
        [SerializeField] bool m_progressiveDisplay = true;
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

        Dictionary<GameObject, AnimationClip> m_animClipCache;
#endregion

#region properties
        public static string version { get { return S(msServerGetVersion()); } }
        public List<MaterialHolder> materialData { get { return m_materialList; } }
        public List<TextureHolder> textureData { get { return m_textureList; } }
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
            catch (Exception e) { Debug.LogError(e); }
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
            try
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
                    case MessageType.Query:
                        OnRecvQuery((QueryMessage)data);
                        break;
                    default:
                        break;
                }
            }
            catch (Exception e) { Debug.LogError(e); }
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

                Record rec = null;
                if (id != 0 && m_hostObjects.TryGetValue(id, out rec))
                {
                    if (rec.go != null)
                        DestroyImmediate(rec.go);
                    m_hostObjects.Remove(id);
                }
                else if (m_clientObjects.TryGetValue(path, out rec))
                {
                    if (rec.go != null)
                        DestroyImmediate(rec.go);
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

#if UNITY_EDITOR
                if (!EditorApplication.isPlaying)
                {
                    // force recalculate skinning
                    foreach (var rec in m_clientObjects)
                    {
                        if (rec.Value.editMesh)
                        {
                            var go = rec.Value.go;
                            if (go != null && go.activeInHierarchy)
                            {
                                go.SetActive(false); // 
                                go.SetActive(true);  // force recalculate skinned mesh on editor. I couldn't find better way...
                            }
                        }
                    }
                }
#endif

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

            // sync textures
            try
            {
                int numTextures = scene.numTextures;
                if (numTextures > 0)
                    UpdateTextures(scene);
            }
            catch (Exception e) { Debug.LogError(e); }

            // materials
            try
            {
                int numMaterials = scene.numMaterials;
                if (numMaterials > 0)
                    UpdateMaterials(scene);
            }
            catch (Exception e) { Debug.LogError(e); }

            // objects
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
            catch (Exception e) { Debug.LogError(e); }

#if UNITY_2018_1_OR_NEWER
            // constraints
            try
            {
                int numConstraints = scene.numConstraints;
                for (int i = 0; i < numConstraints; ++i)
                    UpdateConstraint(scene.GetConstraint(i));
            }
            catch (Exception e) { Debug.LogError(e); }
#endif
            // animations
            try
            {
                int numClips = scene.numAnimationClips;
                for (int i = 0; i < numClips; ++i)
                    UpdateAnimation(scene.GetAnimationClip(i));
            }
            catch (Exception e) { Debug.LogError(e); }

            // update references
            {
                foreach (var pair in m_clientObjects)
                {
                    var dstrec = pair.Value;
                    if (dstrec.reference == null)
                        continue;

                    var dstgo = dstrec.go;
                    if (dstgo == null)
                        continue;

                    Record srcrec = null;
                    if(m_clientObjects.TryGetValue(dstrec.reference, out srcrec))
                    {
                        var srcgo = srcrec.go;
                        if (srcgo != null)
                            UpdateReference(dstgo, srcgo);
                    }
                }
            }

            if(m_progressiveDisplay)
                ForceRepaint();
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

        public Texture2D FindTexture(int id)
        {
            if (id < 0)
                return null;
            var rec = m_textureList.Find(a => a.id == id);
            return rec != null ? rec.texture : null;
        }

        public Material FindMaterial(int id)
        {
            if (id < 0)
                return null;
            var rec = m_materialList.Find(a => a.id == id);
            return rec != null ? rec.material : null;
        }


        void UpdateTextures(SceneData scene)
        {
            MakeSureAssetDirectoryExists();
            string assetDir = "Assets/" + m_assetExportPath;

            int numTextures = scene.numTextures;
            for (int i = 0; i < numTextures; ++i)
            {
                Texture2D texture = null;

                var src = scene.GetTexture(i);
                var format = src.format;
                if (format == TextureFormat.RawFile)
                {
#if UNITY_EDITOR
                    string path = assetDir + "/" + src.name;
                    src.WriteToFile(path);
                    AssetDatabase.ImportAsset(path);
                    texture = AssetDatabase.LoadAssetAtPath<Texture2D>(path);
#endif
                }
                else
                {
                    texture = new Texture2D(src.width, src.height, ToUnityTextureFormat(src.format), false);
                    texture.LoadRawTextureData(src.dataPtr, src.sizeInByte);
                    texture.Apply();
#if UNITY_EDITOR
                    string path = assetDir + "/" + src.name + ".asset";
                    CreateAsset(texture, path);
#endif
                }

                if (texture != null)
                {
                    int id = src.id;
                    var dst = m_textureList.Find(a => a.id == id);
                    if (dst == null)
                    {
                        dst = new TextureHolder();
                        dst.id = id;
                        m_textureList.Add(dst);
                    }
                    dst.texture = texture;
                }
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
                    if (src.id != dst.id || src.name != dst.name || src.color != dst.color)
                    {
                        needsUpdate = true;
                        break;
                    }
                }
            }

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

                    var tmp = CreateDefaultMaterial();
                    tmp.name = src.name;
                    tmp.color = src.color;
                    dst.material = tmp;
                }
                dst.name = src.name;
                dst.color = src.color;

                if (dst.material.name == src.name)
                {
                    var flags = src.flags;

                    // base color
                    if (flags.hasColor)
                        dst.material.color = dst.color;

                    if (flags.hasColorMap)
                    {
                        var mainTex = FindTexture(src.colorMap);
                        if (mainTex != null)
                            dst.material.mainTexture = mainTex;
                    }

                    // emission
                    const string _EmissionColor = "_EmissionColor";
                    const string _EmissionMap = "_EmissionMap";
                    const string _EMISSION = "_EMISSION";

                    if (flags.hasEmission && dst.material.HasProperty(_EmissionColor))
                    {
                        var emission = src.emission;
                        dst.material.SetColor(_EmissionColor, emission);

                        if (emission != Color.black)
                        {
                            if (dst.material.globalIlluminationFlags == MaterialGlobalIlluminationFlags.EmissiveIsBlack)
                            {
                                dst.material.globalIlluminationFlags = MaterialGlobalIlluminationFlags.RealtimeEmissive;
                                dst.material.EnableKeyword(_EMISSION);
                            }
                        }
                        else
                        {
                            if (dst.material.globalIlluminationFlags == MaterialGlobalIlluminationFlags.RealtimeEmissive)
                            {
                                dst.material.globalIlluminationFlags = MaterialGlobalIlluminationFlags.EmissiveIsBlack;
                                dst.material.DisableKeyword(_EMISSION);
                            }
                        }
                    }

                    if (flags.hasEmissionMap && dst.material.HasProperty(_EmissionMap))
                    {
                        var emissionTex = FindTexture(src.emissionMap);
                        if (emissionTex != null)
                            dst.material.SetTexture(_EmissionMap, emissionTex);
                    }

                    // metallic
                    const string _Metallic = "_Metallic";
                    const string _Glossiness = "_Glossiness";
                    const string _MetallicGlossMap = "_MetallicGlossMap";
                    const string _METALLICGLOSSMAP = "_METALLICGLOSSMAP";

                    if (flags.hasMetallic && dst.material.HasProperty(_Metallic))
                        dst.material.SetFloat(_Metallic, src.metalic);

                    if (flags.hasSmoothness && dst.material.HasProperty(_Glossiness))
                        dst.material.SetFloat(_Glossiness, src.smoothness);

                    if (flags.hasMetallicMap && dst.material.HasProperty(_MetallicGlossMap))
                    {
                        var metallicTex = FindTexture(src.metallicMap);
                        if (metallicTex != null)
                        {
                            dst.material.EnableKeyword(_METALLICGLOSSMAP);
                            dst.material.SetTexture(_MetallicGlossMap, metallicTex);
                        }
                        else
                        {
                            dst.material.DisableKeyword(_METALLICGLOSSMAP);
                        }
                    }

                    // normal map
                    const string _BumpMap = "_BumpMap";
                    const string _NORMALMAP = "_NORMALMAP";

                    if (flags.hasNormalMap && dst.material.HasProperty(_BumpMap))
                    {
                        var normalTex = FindTexture(src.normalMap);
                        if (normalTex != null)
                        {
                            dst.material.EnableKeyword(_NORMALMAP);
                            dst.material.SetTexture(_BumpMap, normalTex);
                        }
                        else
                        {
                            dst.material.DisableKeyword(_NORMALMAP);
                        }
                    }
                }

                newlist.Add(dst);
            }

            if (needsUpdate) {
                m_materialList = newlist;
                ReassignMaterials();
            }
        }

        void UpdateMesh(MeshData data)
        {
            var trans = UpdateTransform(data.transform);
            if (trans == null || !m_syncMeshes)
                return;

            var data_trans = data.transform;
            var data_id = data_trans.id;
            var path = data_trans.path;

            Record rec = null;
            if (!m_clientObjects.TryGetValue(path, out rec))
                m_hostObjects.TryGetValue(data_id, out rec);
            if (rec == null)
            {
                Debug.LogError("Something is wrong");
                return;
            }
            else if(rec.reference != null)
            {
                // update later on UpdateReference()
                return;
            }

            var target = rec.go.GetComponent<Transform>();
            var go = target.gameObject;

            bool activeInHierarchy = go.activeInHierarchy;
            if (!activeInHierarchy && !data.flags.hasPoints)
                return;


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

                if (flags.hasIndices)
                {
                    var split = data.GetSplit(si);
                    if (split.numPoints == 0 || split.numIndices == 0)
                    {
                        rec.editMesh = null;
                    }
                    else
                    {
                        rec.editMesh = CreateEditedMesh(data, split);
                        rec.editMesh.name = si == 0 ? target.name : target.name + "[" + si + "]";
                    }
                }

                var smr = GetOrAddSkinnedMeshRenderer(t.gameObject, si > 0);
                if (smr != null)
                {
                    if (flags.hasIndices)
                    {
                        var collider = t.GetComponent<MeshCollider>();
                        bool updateCollider = m_updateMeshColliders && collider != null &&
                            (collider.sharedMesh == null || collider.sharedMesh == smr.sharedMesh);

                        {
                            var old = smr.sharedMesh;
                            smr.sharedMesh = null;
                            DestroyIfNotAsset(old);
                            old = null;
                        }

                        if (updateCollider)
                            collider.sharedMesh = rec.editMesh;

                        bool updateWhenOffscreen = false;
                        if (skinned)
                        {
                            // create bones
                            var bonePaths = data.GetBonePaths();
                            var bones = new Transform[data.numBones];
                            for (int bi = 0; bi < bones.Length; ++bi)
                            {
                                bool dummy = false;
                                bones[bi] = FindOrCreateObjectByPath(bonePaths[bi], false, ref dummy);
                            }

                            if (bones.Length > 0)
                            {
                                bool dummy = false;
                                var root = FindOrCreateObjectByPath(data.rootBonePath, false, ref dummy);
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
                    }

                    if (flags.hasBlendshapeWeights)
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
                if (renderer != null && m_syncVisibility)
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

        PinnedList<int> m_tmpI = new PinnedList<int>();
        PinnedList<Vector2> m_tmpV2 = new PinnedList<Vector2>();
        PinnedList<Vector3> m_tmpV3 = new PinnedList<Vector3>();
        PinnedList<Vector4> m_tmpV4 = new PinnedList<Vector4>();
        PinnedList<Color> m_tmpC = new PinnedList<Color>();

        Mesh CreateEditedMesh(MeshData data, SplitData split)
        {
            var mesh = new Mesh();
#if UNITY_2017_3_OR_NEWER
            mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
#endif

            var flags = data.flags;
            if (flags.hasPoints)
            {
                m_tmpV3.Resize(split.numPoints);
                data.ReadPoints(m_tmpV3, split);
                mesh.SetVertices(m_tmpV3.List);
            }
            if (flags.hasNormals)
            {
                m_tmpV3.Resize(split.numPoints);
                data.ReadNormals(m_tmpV3, split);
                mesh.SetNormals(m_tmpV3.List);
            }
            if (flags.hasTangents)
            {
                m_tmpV4.Resize(split.numPoints);
                data.ReadTangents(m_tmpV4, split);
                mesh.SetTangents(m_tmpV4.List);
            }
            if (flags.hasUV0)
            {
                m_tmpV2.Resize(split.numPoints);
                data.ReadUV0(m_tmpV2, split);
                mesh.SetUVs(0, m_tmpV2.List);
            }
            if (flags.hasUV1)
            {
                m_tmpV2.Resize(split.numPoints);
                data.ReadUV1(m_tmpV2, split);
                mesh.SetUVs(1, m_tmpV2.List);
            }
            if (flags.hasColors)
            {
                m_tmpC.Resize(split.numPoints);
                data.ReadColors(m_tmpC, split);
                mesh.SetColors(m_tmpC.List);
            }
            if (flags.hasBones)
            {
                var tmpW = new PinnedList<BoneWeight>();
                tmpW.Resize(split.numPoints);
                data.ReadBoneWeights(tmpW, split);
                mesh.bindposes = data.bindposes;
                mesh.boneWeights = tmpW.Array;
                tmpW.Dispose();
            }
            if(flags.hasIndices)
            {
                mesh.subMeshCount = split.numSubmeshes;
                for (int smi = 0; smi < mesh.subMeshCount; ++smi)
                {
                    var submesh = split.GetSubmesh(smi);
                    var topology = submesh.topology;

                    m_tmpI.Resize(submesh.numIndices);
                    submesh.ReadIndices(m_tmpI);

                    if (topology == SubmeshData.Topology.Triangles)
                    {
                        mesh.SetTriangles(m_tmpI.List, smi, false);
                    }
                    else
                    {
                        var mt = MeshTopology.Points;
                        switch (topology)
                        {
                            case SubmeshData.Topology.Lines: mt = MeshTopology.Lines; break;
                            case SubmeshData.Topology.Quads: mt = MeshTopology.Quads; break;
                            default: break;
                        }
                        // note: tmpI.Array can't be used because its length is not current size but capacity.
                        mesh.SetIndices(m_tmpI.List.ToArray(), mt, smi, false);
                    }

                }
            }
            if (flags.hasBlendshapes)
            {
                var tmpBSP = new PinnedList<Vector3>(split.numPoints);
                var tmpBSN = new PinnedList<Vector3>(split.numPoints);
                var tmpBST = new PinnedList<Vector3>(split.numPoints);

                int numBlendShapes = data.numBlendShapes;
                for (int bi = 0; bi < numBlendShapes; ++bi)
                {
                    var bsd = data.GetBlendShapeData(bi);
                    var name = bsd.name;
                    var numFrames = bsd.numFrames;
                    for (int fi = 0; fi < numFrames; ++fi)
                    {
                        bsd.ReadPoints(fi, tmpBSP, split);
                        bsd.ReadNormals(fi, tmpBSN, split);
                        bsd.ReadTangents(fi, tmpBST, split);
                        mesh.AddBlendShapeFrame(name, bsd.GetWeight(fi), tmpBSP.Array, tmpBSN.Array, tmpBST.Array);
                    }
                }

                tmpBSP.Dispose();
                tmpBSN.Dispose();
                tmpBST.Dispose();
            }

            mesh.bounds = split.bounds;
            mesh.UploadMeshData(false);
            return mesh;
        }

        void MakeSureAssetDirectoryExists()
        {
#if UNITY_EDITOR
            try
            {
                string assetDir = "Assets/" + m_assetExportPath;
                if (!AssetDatabase.IsValidFolder(assetDir))
                    AssetDatabase.CreateFolder("Assets", m_assetExportPath);
            }
            catch (Exception e) { Debug.LogError(e); }
#endif
        }

        void CreateAsset(UnityEngine.Object obj, string assetPath)
        {
#if UNITY_EDITOR
            try
            {
                string assetDir = "Assets/" + m_assetExportPath;
                if (!AssetDatabase.IsValidFolder(assetDir))
                    AssetDatabase.CreateFolder("Assets", m_assetExportPath);
                AssetDatabase.CreateAsset(obj, SanitizeFileName(assetPath));
            }
            catch (Exception e) { Debug.LogError(e); }
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

            // sync TRS
            if (m_syncTransform)
            {
                trans.localPosition = data.position;
                trans.localRotation = data.rotation;
                trans.localScale = data.scale;
            }

            // visibility
            if (m_syncVisibility)
            {
                trans.gameObject.SetActive(data.visibleHierarchy);

                var smr = trans.GetComponent<SkinnedMeshRenderer>();
                if (smr != null)
                    smr.enabled = data.visible;

                var cam = trans.GetComponent<Camera>();
                if (cam != null)
                    cam.enabled = data.visible;

                var light = trans.GetComponent<Light>();
                if (light != null)
                    light.enabled = data.visible;
            }

            return trans;
        }

        Camera UpdateCamera(CameraData data)
        {
            var trans = UpdateTransform(data.transform);
            if (trans == null || !m_syncCameras)
                return null;

            var cam = trans.GetComponent<Camera>();
            if(cam == null)
                cam = trans.gameObject.AddComponent<Camera>();

            cam.orthographic = data.orthographic;

            float fov = data.fov;
            if (fov > 0.0f)
                cam.fieldOfView = fov;

            float nearClipPlane = data.nearClipPlane;
            float farClipPlane = data.farClipPlane;
            if (nearClipPlane > 0.0f && farClipPlane > 0.0f)
            {
                cam.nearClipPlane = data.nearClipPlane;
                cam.farClipPlane = data.farClipPlane;
            }

            if (m_syncVisibility)
                cam.enabled = data.transform.visible;
            return cam;
        }

        Light UpdateLight(LightData data)
        {
            var trans = UpdateTransform(data.transform);
            if (trans == null || !m_syncLights)
                return null;

            var lt = trans.GetComponent<Light>();
            if (lt == null)
                lt = trans.gameObject.AddComponent<Light>();

            lt.type = data.type;
            lt.color = data.color;
            lt.intensity = data.intensity;
            if (data.range > 0.0f)
                lt.range = data.range;
            if (data.spotAngle > 0.0f)
                lt.spotAngle = data.spotAngle;
            if (m_syncVisibility)
                lt.enabled = data.transform.visible;
            return lt;
        }

        void UpdateReference(GameObject dstgo, GameObject srcgo)
        {
            var srcsmr = srcgo.GetComponent<SkinnedMeshRenderer>();
            if (srcsmr != null)
            {
                var dstsmr = dstgo.GetComponent<SkinnedMeshRenderer>();
                if (dstsmr == null)
                    dstsmr = dstgo.AddComponent<SkinnedMeshRenderer>();

                var mesh = srcsmr.sharedMesh;
                dstsmr.sharedMesh = mesh;
                dstsmr.sharedMaterials = srcsmr.sharedMaterials;
                dstsmr.bones = srcsmr.bones;
                dstsmr.rootBone = srcsmr.rootBone;
                dstsmr.updateWhenOffscreen = srcsmr.updateWhenOffscreen;
                if (mesh != null)
                {
                    int blendShapeCount = mesh.blendShapeCount;
                    for (int bi = 0; bi < blendShapeCount; ++bi)
                        dstsmr.SetBlendShapeWeight(bi, srcsmr.GetBlendShapeWeight(bi));
                }

#if UNITY_EDITOR
                if (!EditorApplication.isPlaying)
                {
                    dstgo.SetActive(false); // 
                    dstgo.SetActive(true);  // force recalculate skinned mesh on editor
                }
#endif
            }
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
            var trans = FindOrCreateObjectByPath(data.path, true, ref dummy);
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
                    s.sourceTransform = FindOrCreateObjectByPath(data.GetSourcePath(si), true, ref dummy);
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

        void UpdateAnimation(AnimationClipData clipData)
        {
#if UNITY_EDITOR
            int numAnimations = clipData.numAnimations;
            for (int ai = 0; ai < numAnimations; ++ai)
            {
                AnimationData data = clipData.GetAnimation(ai);

                var path = data.path;
                bool dummy = false;
                var target = FindOrCreateObjectByPath(path, true, ref dummy);
                if (target == null)
                    return;

                Transform root = target;
                while (root.parent != null && root.parent != m_rootObject)
                    root = root.parent;

                Animator animator = null;
                AnimationClip clip = null;

                if (m_animClipCache == null)
                    m_animClipCache = new Dictionary<GameObject, AnimationClip>();
                else if (m_animClipCache.ContainsKey(root.gameObject))
                    clip = m_animClipCache[root.gameObject];

                animator = root.GetComponent<Animator>();
                if (animator == null)
                {
                    animator = root.gameObject.AddComponent<Animator>();
                }
                else if (clip == null && animator.runtimeAnimatorController != null)
                {
                    var clips = animator.runtimeAnimatorController.animationClips;
                    if (clips != null && clips.Length > 0)
                    {
                        // note: this is extremely slow. m_animClipTable exists to cache the result and avoid frequent call.
                        var tmp = animator.runtimeAnimatorController.animationClips[0];
                        if (tmp != null)
                        {
                            clip = tmp;
                            m_animClipCache[root.gameObject] = tmp;
                        }
                    }
                }

                if (clip == null)
                {
                    clip = new AnimationClip();
                    var clipName = clipData.name;
                    if(clipName.Length > 0)
                        clipName = root.name + "_" + clipName;
                    else
                        clipName = root.name;

                    var assetPath = "Assets/" + m_assetExportPath + "/" + SanitizeFileName(clipName) + ".anim";
                    CreateAsset(clip, assetPath);
                    animator.runtimeAnimatorController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPathWithClip(assetPath + ".controller", clip);
                    m_animClipCache[root.gameObject] = clip;
                }

                var animPath = path.Replace("/" + root.name, "");
                if (animPath.Length > 0 && animPath[0] == '/')
                {
                    animPath = animPath.Remove(0, 1);
                }

                InterpolationMethod im = SmoothInterpolation;
                //switch (m_animtionInterpolation)
                //{
                //    case InterpolationType.Linear:
                //        im = LinearInterpolation;
                //        break;
                //    default:
                //        im = SmoothInterpolation;
                //        break;
                //}
                data.ExportToClip(clip, root.gameObject, target.gameObject, animPath, im);
            }

            if (m_animClipCache != null)
            {
                // call EnsureQuaternionContinuity() to smooth rotation
                foreach (var kvp in m_animClipCache)
                    kvp.Value.EnsureQuaternionContinuity();
            }

            // clear clip cache
            m_animClipCache = null;
#endif
        }

        public void ReassignMaterials()
        {
            foreach (var rec in m_clientObjects)
                AssignMaterials(rec.Value);
            foreach (var rec in m_hostObjects)
                AssignMaterials(rec.Value);
        }

        void AssignMaterials(Record rec)
        {
            if (rec.go == null) { return; }

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
                if (r == null) { continue; }

                int submeshCount = submeshCounts[i];
                var prev = r.sharedMaterials;
                var materials = new Material[submeshCount];
                bool changed = false;

                for (int j = 0; j < submeshCount; ++j)
                {
                    if (j < prev.Length && prev[j] != null)
                        materials[j] = prev[j];

                    var mid = materialIDs[mi++];
                    var material = FindMaterial(mid);
                    if (material != null)
                    {
                        if (materials[j] != material)
                        {
                            materials[j] = material;
                            changed = true;
                        }
                    }
                    else
                    {
                        if (materials[j] == null)
                        {
                            var tmp = CreateDefaultMaterial();
                            tmp.name = "DefaultMaterial";
                            materials[j] = tmp;
                            changed = true;
                        }
                    }
                }

                if (changed)
                    r.sharedMaterials = materials;
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
            Transform t = m_rootObject;
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

        public static Material CreateDefaultMaterial()
        {
            return new Material(Shader.Find("Standard"));
        }

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

        string BuildPath(Transform t)
        {
            var parent = t.parent;
            if (parent != null && parent != m_rootObject)
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
        bool ServeTexture(Texture2D v, GetMessage mes)
        {
            var data = TextureData.Create();
            data.name = v.name;
            // todo
            msServerServeTexture(m_server, data);
            return true;
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
                    dst.SetBonePaths(this, smr.bones);
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
            ForceRepaint();

#if UNITY_2017_1_OR_NEWER
            ScreenCapture.CaptureScreenshot("screenshot.png");
#else
            Application.CaptureScreenshot("screenshot.png");
#endif
            // actual capture will be done at end of frame. not done immediately.
            // just set flag now.
            m_captureScreenshotInProgress = true;
        }

        void OnRecvQuery(QueryMessage data)
        {
            switch (data.queryType)
            {
                case QueryMessage.QueryType.ClientName:
                    data.AddResponseText("Unity");
                    break;
                case QueryMessage.QueryType.AllNodes:
                    {
                        var roots = UnityEngine.SceneManagement.SceneManager.GetActiveScene().GetRootGameObjects();
                        foreach (var go in roots)
                            data.AddResponseText(BuildPath(go.GetComponent<Transform>()));
                    }
                    break;
                case QueryMessage.QueryType.RootNodes:
                    {
                        var objects = FindObjectsOfType<Transform>();
                        foreach (var go in objects)
                            data.AddResponseText(BuildPath(go.GetComponent<Transform>()));
                    }
                    break;
                default:
                    break;
            }
            data.FinishRespond();
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

        public void ExportMaterials()
        {
            if (m_materialList == null)
                return;

            string assetDir = "Assets/" + m_assetExportPath;
            foreach (var m in m_materialList)
            {
                var material = m.material;
                if (AssetDatabase.GetAssetPath(material) == "")
                {
                    string path = assetDir + "/" + material.name + ".mat";
                    CreateAsset(material, path);
                }
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
                var assetPath = "Assets/" + m_assetExportPath + "/" + mf.sharedMesh.name + ".asset";
                CreateAsset(mf.sharedMesh, assetPath);
                if (m_logging)
                {
                    Debug.Log("exported mesh " + assetPath);
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

            m_tmpI.Dispose();
            m_tmpV2.Dispose();
            m_tmpV3.Dispose();
            m_tmpV4.Dispose();
            m_tmpC.Dispose();
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
