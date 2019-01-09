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
    public delegate void SceneHandler();
    public delegate void UpdateAudioHandler(AudioClip audio, AudioData data);
    public delegate void UpdateTextureHandler(Texture2D tex, TextureData data);
    public delegate void UpdateMaterialHandler(Material mat, MaterialData data);
    public delegate void UpdateEntityHandler(GameObject obj, TransformData data);
    public delegate void UpdateAnimationHandler(AnimationClip anim, AnimationClipData data);
    public delegate void DeleteEntityHandler(GameObject obj);

    [ExecuteInEditMode]
    public class MeshSyncServer : MonoBehaviour, ISerializationCallbackReceiver
    {
        #region Events
        public event SceneHandler onSceneUpdateBegin;
        public event UpdateAudioHandler onUpdateAudio;
        public event UpdateTextureHandler onUpdateTexture;
        public event UpdateMaterialHandler onUpdateMaterial;
        public event UpdateEntityHandler onUpdateEntity;
        public event UpdateAnimationHandler onUpdateAnimation;
        public event DeleteEntityHandler onDeleteEntity;
        public event SceneHandler onSceneUpdateEnd;
        #endregion


        #region Types
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
                int numSubmeshes = md.numSubmeshes;
                if (numSubmeshes == 0)
                    return false;

                var mids = new int[numSubmeshes];
                for (int i = 0; i < numSubmeshes; ++i)
                    mids[i] = md.GetSubmesh(i).materialID;

                int numSplits = md.numSplits;
                var scs = new int[numSplits];
                for (int i = 0; i < numSplits; ++i)
                    scs[i] = md.GetSplit(i).numSubmeshes;

                bool ret = !materialIDs.SequenceEqual(mids) || !submeshCounts.SequenceEqual(scs);
                materialIDs = mids;
                submeshCounts = scs;
                return ret;
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

        [Serializable]
        public class AudioHolder
        {
            public int id;
            public string name;
            public AudioClip audio;
        }

        // thanks: http://techblog.sega.jp/entry/2016/11/28/100000
        public class AnimationCurveKeyReducer
        {
            static public void DoReduction(AnimationCurve curve, float eps = 0.001f)
            {
                if (curve.keys.Length <= 2)
                    return;

                var indices = GetDeleteKeyIndex(curve.keys, eps).ToArray();
                foreach (var idx in indices.Reverse())
                    curve.RemoveKey(idx);
            }

            static IEnumerable<int> GetDeleteKeyIndex(Keyframe[] keys, float eps)
            {
                for (int k = 0, i = 1; i < keys.Length - 1; i++)
                {
                    if (IsInterpolationValue(keys[k], keys[i + 1], keys[i], eps))
                        yield return i;
                    else
                        k = i;
                }
            }

            static bool IsInterpolationValue(Keyframe key1, Keyframe key2, Keyframe comp, float eps)
            {
                var val1 = GetValueFromTime(key1, key2, comp.time);
                if (eps < System.Math.Abs(comp.value - val1))
                    return false;

                var time = key1.time + (comp.time - key1.time) * 0.5f;
                val1 = GetValueFromTime(key1, comp, time);
                var val2 = GetValueFromTime(key1, key2, time);

                return System.Math.Abs(val2 - val1) <= eps ? true : false;
            }

            static float GetValueFromTime(Keyframe key1, Keyframe key2, float time)
            {
                float t;
                float a, b, c;
                float kd, vd;

                if (key1.outTangent == Mathf.Infinity)
                    return key1.value;

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


        #region Fields
        [SerializeField] int m_serverPort = 8080;
        [SerializeField] string m_assetDir = "MeshSyncAssets";
        [SerializeField] Transform m_rootObject;
        [Space(10)]
        [SerializeField] bool m_syncVisibility = true;
        [SerializeField] bool m_syncTransform = true;
        [SerializeField] bool m_syncCameras = true;
        [SerializeField] bool m_syncLights = true;
        [SerializeField] bool m_syncMeshes = true;
        [SerializeField] bool m_syncPoints = true;
        [Space(10)]
        [SerializeField] bool m_updateMeshColliders = true;
        [SerializeField] bool m_findMaterialFromAssets = true;
        [SerializeField] bool m_trackMaterialAssignment = true;
        [SerializeField] InterpolationType m_animtionInterpolation = InterpolationType.Smooth;
        [Space(10)]
        [SerializeField] bool m_progressiveDisplay = true;
        [SerializeField] bool m_logging = true;

        [HideInInspector] [SerializeField] List<MaterialHolder> m_materialList = new List<MaterialHolder>();
        [HideInInspector] [SerializeField] List<TextureHolder> m_textureList = new List<TextureHolder>();
        [HideInInspector] [SerializeField] List<AudioHolder> m_audioList = new List<AudioHolder>();

        Server m_server;
        Server.MessageHandler m_handler;
        bool m_requestRestartServer = false;
        bool m_captureScreenshotInProgress = false;
        bool m_needReassignMaterials = false;

        Dictionary<string, EntityRecord> m_clientObjects = new Dictionary<string, EntityRecord>();
        Dictionary<int, EntityRecord> m_hostObjects = new Dictionary<int, EntityRecord>();
        Dictionary<GameObject, int> m_objIDTable = new Dictionary<GameObject, int>();

        [HideInInspector] [SerializeField] string[] m_clientObjects_keys;
        [HideInInspector] [SerializeField] EntityRecord[] m_clientObjects_values;
        [HideInInspector] [SerializeField] int[] m_hostObjects_keys;
        [HideInInspector] [SerializeField] EntityRecord[] m_hostObjects_values;
        [HideInInspector] [SerializeField] GameObject[] m_objIDTable_keys;
        [HideInInspector] [SerializeField] int[] m_objIDTable_values;
        [HideInInspector] [SerializeField] int m_objIDSeed = 0;
        #endregion


        #region Properties
        public static string version { get { return Server.version; } }
        public int serverPort
        {
            get { return m_serverPort; }
            set { m_serverPort = value; }
        }
        public string assetDir
        {
            get { return m_assetDir; }
            set { m_assetDir = value; }
        }
        public string assetPath
        {
            get { return "Assets/" + m_assetDir; }
        }
        public string httpFileRootPath
        {
            get { return Application.streamingAssetsPath + "/MeshSyncServerRoot"; }
        }
        public Transform rootObject
        {
            get { return m_rootObject; }
            set { m_rootObject = value; }
        }
        public List<MaterialHolder> materialData { get { return m_materialList; } }
        public List<TextureHolder> textureData { get { return m_textureList; } }
        #endregion


        #region Impl
#if UNITY_EDITOR
        [MenuItem("GameObject/MeshSync/Create Server", false, 10)]
        public static void CreateMeshSyncServer(MenuCommand menuCommand)
        {
            var go = new GameObject();
            go.name = "MeshSyncServer";
            var mss = go.AddComponent<MeshSyncServer>();
            mss.rootObject = go.GetComponent<Transform>();
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
                        dic[keys[i]] = values[i];
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

            var settings = ServerSettings.defaultValue;
            settings.port = (ushort)m_serverPort;
            m_server = Server.Start(ref settings);
            m_server.fileRootPath = httpFileRootPath;
            m_handler = OnServerMessage;
#if UNITY_EDITOR
            EditorApplication.update += PollServerEvents;
            SceneView.onSceneGUIDelegate += OnSceneViewGUI;
#endif
        }

        void StopServer()
        {
            if (m_server)
            {
#if UNITY_EDITOR
                EditorApplication.update -= PollServerEvents;
                SceneView.onSceneGUIDelegate -= OnSceneViewGUI;
#endif
                m_server.Stop();
                m_server = default(Server);
            }
        }


        #region MessageHandlers
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
                m_server.screenshotPath = "screenshot.png";
            }

            if (m_server.numMessages > 0)
                m_server.ProcessMessages(m_handler);
        }

        void OnServerMessage(MessageType type, IntPtr data)
        {
            Try(() =>
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
            });
        }

        void OnRecvGet(GetMessage mes)
        {
            m_server.BeginServe();
            foreach (var mr in FindObjectsOfType<Renderer>())
                ServeMesh(mr, mes);
            foreach (var mat in m_materialList)
                ServeMaterial(mat.material, mes);
            m_server.EndServe();

            if (m_logging)
                Debug.Log("MeshSyncServer: served");
        }

        void OnRecvDelete(DeleteMessage mes)
        {
            int numEntities = mes.numEntities;
            for (int i = 0; i < numEntities; ++i)
            {
                var identifier = mes.GetEntity(i);
                var id = identifier.id;
                var path = identifier.name;

                GameObject target = null;
                EntityRecord rec = null;
                if (id != 0 && m_hostObjects.TryGetValue(id, out rec))
                {
                    if (rec.go != null)
                        target = rec.go;
                    m_hostObjects.Remove(id);
                }
                else if (m_clientObjects.TryGetValue(path, out rec))
                {
                    if (rec.go != null)
                        target = rec.go;
                    m_clientObjects.Remove(path);
                }

                if (target != null && !IsAsset(target))
                {
                    if (onDeleteEntity != null)
                        onDeleteEntity.Invoke(target);
                    DestroyImmediate(target);
                }
            }

            int numMaterials = mes.numMaterials;
            for (int i = 0; i < numMaterials; ++i)
            {
                var id = mes.GetMaterial(i).id;
                m_materialList.RemoveAll(v => v.id == id);
            }
        }

        void OnRecvFence(FenceMessage mes)
        {
            if(mes.type == FenceMessage.FenceType.SceneBegin)
            {
                if (onSceneUpdateBegin != null)
                    onSceneUpdateBegin.Invoke();
            }
            else if(mes.type == FenceMessage.FenceType.SceneEnd)
            {
                // resolve references
                foreach (var pair in m_clientObjects)
                {
                    var dstrec = pair.Value;
                    if (dstrec.reference == null || dstrec.go == null)
                        continue;

                    EntityRecord srcrec = null;
                    if (m_clientObjects.TryGetValue(dstrec.reference, out srcrec) && srcrec.go != null)
                        UpdateReference(dstrec.go, srcrec.go);
                }

                // sort objects by index
                {
                    var rec = m_clientObjects.Values.OrderBy(v => v.index);
                    foreach (var r in rec)
                    {
                        if (r.go != null)
                            r.go.GetComponent<Transform>().SetSiblingIndex(r.index + 1000);
                    }
                }

                // reassign materials
                m_materialList = m_materialList.OrderBy(v => v.index).ToList();
                if (m_needReassignMaterials)
                {
                    ReassignMaterials();
                    m_needReassignMaterials = false;
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

                m_server.NotifyPoll(PollMessage.PollType.SceneUpdate);
                if (onSceneUpdateEnd != null)
                    onSceneUpdateEnd.Invoke();
            }
        }

        void OnRecvText(TextMessage mes)
        {
            mes.Print();
        }

        void OnRecvSet(SetMessage mes)
        {
            MakeSureAssetDirectoryExists();
            var scene = mes.scene;

            // assets
            Try(() =>
            {
                int numAssets = scene.numAssets;
                if (numAssets > 0)
                {
                    for (int i = 0; i < numAssets; ++i)
                    {
                        var asset = scene.GetAsset(i);
                        switch (asset.type)
                        {
                            case AssetType.File:
                                ((FileAssetData)asset).WriteToFile(assetPath + "/" + asset.name);
                                break;
                            case AssetType.Audio:
                                UpdateAudio((AudioData)asset);
                                break;
                            case AssetType.Texture:
                                UpdateTexture((TextureData)asset);
                                break;
                            case AssetType.Material:
                                UpdateMaterial((MaterialData)asset);
                                break;
                            case AssetType.Animation:
                                UpdateAnimation((AnimationClipData)asset);
                                break;
                            default:
                                if(m_logging)
                                    Debug.Log("unknown asset: " + asset.name);
                                break;
                        }
                    }
                }
            });

            // objects
            Try(() =>
            {
                int numObjects = scene.numEntities;
                for (int i = 0; i < numObjects; ++i)
                {
                    Component dst = null;
                    var src = scene.GetEntity(i);
                    switch (src.type)
                    {
                        case TransformData.Type.Transform:
                            dst = UpdateTransform(src);
                            break;
                        case TransformData.Type.Camera:
                            dst = UpdateCamera((CameraData)src);
                            break;
                        case TransformData.Type.Light:
                            dst = UpdateLight((LightData)src);
                            break;
                        case TransformData.Type.Mesh:
                            dst = UpdateMesh((MeshData)src);
                            break;
                        case TransformData.Type.Points:
                            dst = UpdatePoints((PointsData)src);
                            break;
                    }

                    if (dst != null && onUpdateEntity != null)
                        onUpdateEntity.Invoke(dst.gameObject, src);
                }
            });

#if UNITY_2018_1_OR_NEWER
            // constraints
            Try(() =>
            {
                int numConstraints = scene.numConstraints;
                for (int i = 0; i < numConstraints; ++i)
                    UpdateConstraint(scene.GetConstraint(i));
            });
#endif

            if(m_progressiveDisplay)
                ForceRepaint();
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
                    data.AddResponseText("Unity " + Application.unityVersion);
                    break;
                case QueryMessage.QueryType.RootNodes:
                    {
                        var roots = UnityEngine.SceneManagement.SceneManager.GetActiveScene().GetRootGameObjects();
                        foreach (var go in roots)
                            data.AddResponseText(BuildPath(go.GetComponent<Transform>()));
                    }
                    break;
                case QueryMessage.QueryType.AllNodes:
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
        #endregion


        #region Misc
        //[MethodImpl(MethodImplOptions.AggressiveInlining)]
        bool Try(Action act)
        {
            try
            {
                act.Invoke();
                return true;
            }
            catch (Exception e)
            {
                if (m_logging)
                    Debug.LogError(e);
                return false;
            }
        }

        public void MakeSureAssetDirectoryExists()
        {
#if UNITY_EDITOR
            Try(()=> {
                if (!AssetDatabase.IsValidFolder(assetPath))
                    AssetDatabase.CreateFolder("Assets", m_assetDir);
            });
#endif
        }

        bool CreateAsset(UnityEngine.Object obj, string assetPath)
        {
#if UNITY_EDITOR
            return Try(() =>
            {
                AssetDatabase.CreateAsset(obj, Misc.SanitizeFileName(assetPath));
            });
#else
            return false;
#endif
        }

        bool IsAsset(UnityEngine.Object obj)
        {
#if UNITY_EDITOR
            if (AssetDatabase.GetAssetPath(obj) != "")
                return true;
#endif
            return false;
        }

        bool DestroyIfNotAsset(UnityEngine.Object obj)
        {
            if (obj != null && !IsAsset(obj))
            {
                DestroyImmediate(obj, false);
                return true;
            }
            return false;
        }

        byte[] EncodeToPNG(Texture2D tex)
        {
#if UNITY_2017_3_OR_NEWER
            return ImageConversion.EncodeToPNG(tex);
#else
            return tex.EncodeToPNG();
#endif
        }
        byte[] EncodeToEXR(Texture2D tex, Texture2D.EXRFlags flags)
        {
#if UNITY_2017_3_OR_NEWER
            return ImageConversion.EncodeToEXR(tex, flags);
#else
            return tex.EncodeToEXR(flags);
#endif
        }

        public string BuildPath(Transform t)
        {
            var parent = t.parent;
            if (parent != null && parent != m_rootObject)
                return BuildPath(parent) + "/" + t.name;
            else
                return "/" + t.name;
        }

        public Texture2D FindTexture(int id)
        {
            if (id == Misc.InvalidID)
                return null;
            var rec = m_textureList.Find(a => a.id == id);
            return rec != null ? rec.texture : null;
        }

        public Material FindMaterial(int id)
        {
            if (id == Misc.InvalidID)
                return null;
            var rec = m_materialList.Find(a => a.id == id);
            return rec != null ? rec.material : null;
        }

        int GetMaterialIndex(Material mat)
        {
            if (mat == null)
                return Misc.InvalidID;

            for (int i = 0; i < m_materialList.Count; ++i)
            {
                if (m_materialList[i].material == mat)
                    return i;
            }

            int ret = m_materialList.Count;
            var tmp = new MaterialHolder();
            tmp.name = mat.name;
            tmp.material = mat;
            tmp.id = ret + 1;
            m_materialList.Add(tmp);
            return ret;
        }

        public AudioClip FindAudio(int id)
        {
            if (id == Misc.InvalidID)
                return null;
            var rec = m_audioList.Find(a => a.id == id);
            return rec != null ? rec.audio : null;
        }

        int GetObjectlID(GameObject go)
        {
            if (go == null)
                return Misc.InvalidID;

            int ret;
            if (m_objIDTable.ContainsKey(go))
            {
                ret = m_objIDTable[go];
            }
            else
            {
                ret = ++m_objIDSeed;
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
                if (name.Length == 0)
                    continue;
                t = FindOrCreateObjectByName(t, name, createIfNotExist, ref created);
                if (t == null)
                    break;
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

        static Material CreateDefaultMaterial()
        {
            var ret = new Material(Shader.Find("Standard"));
            return ret;
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
        #endregion



        #region ReceiveScene
        void UpdateAudio(AudioData src)
        {
            AudioClip ac = null;

            var format = src.format;
            if (format == AudioFormat.RawFile)
            {
                var dstPath = assetPath + "/" + src.name;
                src.WriteToFile(dstPath);
#if UNITY_EDITOR
                AssetDatabase.ImportAsset(dstPath);
                ac = AssetDatabase.LoadAssetAtPath<AudioClip>(dstPath);
                if (ac != null)
                {
                    var importer = (AudioImporter)AssetImporter.GetAtPath(dstPath);
                    if (importer != null)
                    {
                        // nothing todo for now
                    }
                }
#endif
            }
            else
            {
#if UNITY_EDITOR
                var dstPath = assetPath + "/" + src.name + ".wav";
                if(src.ExportAsWave(dstPath))
                {
                    AssetDatabase.ImportAsset(dstPath);
                    ac = AssetDatabase.LoadAssetAtPath<AudioClip>(dstPath);
                    if (ac != null)
                    {
                        var importer = (AudioImporter)AssetImporter.GetAtPath(dstPath);
                        if (importer != null)
                        {
                            // nothing todo for now
                        }
                    }
                }
#endif
                if (ac == null)
                {
                    ac = AudioClip.Create(src.name, src.sampleLength, src.channels, src.frequency, false);
                    ac.SetData(src.samples, 0);
                }
            }

            if (ac != null)
            {
                int id = src.id;
                var dst = m_audioList.Find(a => a.id == id);
                if (dst == null)
                {
                    dst = new AudioHolder();
                    dst.id = id;
                    m_audioList.Add(dst);
                }
                dst.audio = ac;
                if (onUpdateAudio != null)
                    onUpdateAudio.Invoke(ac, src);
            }
        }

        void UpdateTexture(TextureData src)
        {
            Texture2D texture = null;
#if UNITY_EDITOR
            Action<string> doImport = (path) =>
            {
                AssetDatabase.ImportAsset(path);
                texture = AssetDatabase.LoadAssetAtPath<Texture2D>(path);
                if (texture != null)
                {
                    var importer = (TextureImporter)AssetImporter.GetAtPath(path);
                    if (importer != null)
                    {
                        if (src.type == TextureType.NormalMap)
                            importer.textureType = TextureImporterType.NormalMap;
                    }
                }
            };
#endif

            var format = src.format;
            if (format == TextureFormat.RawFile)
            {
#if UNITY_EDITOR
                // write data to file and import
                string path = assetPath + "/" + src.name;
                if (src.WriteToFile(path))
                    doImport(path);
#endif
            }
            else
            {
                texture = new Texture2D(src.width, src.height, Misc.ToUnityTextureFormat(src.format), false);
                texture.name = src.name;
                texture.LoadRawTextureData(src.dataPtr, src.sizeInByte);
                texture.Apply();
#if UNITY_EDITOR
                // encode and write data to file and import
                // (script-generated texture works but can't set texture type such as normal map)
                bool exported = false;
                string path = null;
                switch (src.format)
                {
                    case TextureFormat.Ru8:
                    case TextureFormat.RGu8:
                    case TextureFormat.RGBu8:
                    case TextureFormat.RGBAu8:
                        {
                            path = assetPath + "/" + src.name + ".png";
                            exported = TextureData.WriteToFile(path, EncodeToPNG(texture));
                            break;
                        }
                    case TextureFormat.Rf16:
                    case TextureFormat.RGf16:
                    case TextureFormat.RGBf16:
                    case TextureFormat.RGBAf16:
                        {
                            path = assetPath + "/" + src.name + ".exr";
                            exported = TextureData.WriteToFile(path, EncodeToEXR(texture, Texture2D.EXRFlags.CompressZIP));
                            break;
                        }
                    case TextureFormat.Rf32:
                    case TextureFormat.RGf32:
                    case TextureFormat.RGBf32:
                    case TextureFormat.RGBAf32:
                        {
                            path = assetPath + "/" + src.name + ".exr";
                            exported = TextureData.WriteToFile(path, EncodeToEXR(texture, Texture2D.EXRFlags.OutputAsFloat | Texture2D.EXRFlags.CompressZIP));
                            break;
                        }
                }

                if (exported)
                {
                    texture = null;
                    GC.Collect();

                    doImport(path);
                }
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
                if (onUpdateTexture != null)
                    onUpdateTexture.Invoke(texture, src);
            }
        }


        // keyword strings
        const string _Color = "_Color";
        const string _MainTex = "_MainTex";

        const string _EmissionColor = "_EmissionColor";
        const string _EmissionMap = "_EmissionMap";
        const string _EMISSION = "_EMISSION";

        const string _Metallic = "_Metallic";
        const string _Glossiness = "_Glossiness";
        const string _MetallicGlossMap = "_MetallicGlossMap";
        const string _METALLICGLOSSMAP = "_METALLICGLOSSMAP";

        const string _BumpMap = "_BumpMap";
        const string _NORMALMAP = "_NORMALMAP";

        void UpdateMaterial(MaterialData src)
        {
            var id = src.id;

            var dst = m_materialList.Find(a => a.id == id);
            if (dst == null)
            {
                dst = new MaterialHolder();
                dst.id = id;
                m_materialList.Add(dst);
            }
            if (dst.material == null)
            {
#if UNITY_EDITOR
                if (m_findMaterialFromAssets)
                {
                    var guids = AssetDatabase.FindAssets("t:Material " + src.name);
                    if (guids.Length > 0)
                        dst.material = AssetDatabase.LoadAssetAtPath<Material>(AssetDatabase.GUIDToAssetPath(guids[0]));
                }
#endif
                if (dst.material == null)
                {
                    var shader = Shader.Find(src.shader);
                    if (shader == null)
                        shader = Shader.Find("Standard");
                    dst.material = new Material(shader);
                }

                dst.materialIID = dst.material.GetInstanceID();
                m_needReassignMaterials = true;
            }
            dst.name = src.name;
            dst.index = src.index;
            dst.shader = src.shader;
            dst.color = src.color;

            if (dst.materialIID != dst.material.GetInstanceID())
                return;

            var dstmat = dst.material;
            dstmat.name = src.name;

            int numKeywords = src.numKeywords;
            for (int ki = 0; ki < numKeywords; ++ki)
            {
                var kw = src.GetKeyword(ki);
                if (kw.value)
                    dstmat.EnableKeyword(kw.name);
                else
                    dstmat.DisableKeyword(kw.name);
            }

            int numProps = src.numProperties;
            for (int pi = 0; pi < numProps; ++pi)
            {
                var prop = src.GetProperty(pi);
                var name = prop.name;
                var type = prop.type;
                if (!dstmat.HasProperty(name))
                    continue;

                // todo: handle transparent
                //if (name == _Color)
                //{
                //    var color = prop.vectorValue;
                //    if (color.w > 0.0f && color.w < 1.0f && dstmat.HasProperty("_SrcBlend"))
                //    {
                //        dstmat.SetOverrideTag("RenderType", "Transparent");
                //        dstmat.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.One);
                //        dstmat.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
                //        dstmat.SetInt("_ZWrite", 0);
                //        dstmat.DisableKeyword("_ALPHATEST_ON");
                //        dstmat.DisableKeyword("_ALPHABLEND_ON");
                //        dstmat.EnableKeyword("_ALPHAPREMULTIPLY_ON");
                //        dstmat.renderQueue = (int)UnityEngine.Rendering.RenderQueue.Transparent;
                //    }
                //}
                if (name == _EmissionColor)
                {
                    if (dstmat.globalIlluminationFlags == MaterialGlobalIlluminationFlags.EmissiveIsBlack)
                    {
                        dstmat.globalIlluminationFlags = MaterialGlobalIlluminationFlags.RealtimeEmissive;
                        dstmat.EnableKeyword(_EMISSION);
                    }
                }
                else if (name == _MetallicGlossMap)
                {
                    dstmat.EnableKeyword(_METALLICGLOSSMAP);
                }
                else if (name == _BumpMap)
                {
                    dstmat.EnableKeyword(_NORMALMAP);
                }

                int len = prop.arrayLength;
                switch (type)
                {
                    case MaterialPropertyData.Type.Int:
                        dstmat.SetInt(name, prop.intValue);
                        break;
                    case MaterialPropertyData.Type.Float:
                        if (len == 1)
                            dstmat.SetFloat(name, prop.floatValue);
                        else
                            dstmat.SetFloatArray(name, prop.floatArray);
                        break;
                    case MaterialPropertyData.Type.Vector:
                        if (len == 1)
                            dstmat.SetVector(name, prop.vectorValue);
                        else
                            dstmat.SetVectorArray(name, prop.vectorArray);
                        break;
                    case MaterialPropertyData.Type.Matrix:
                        if (len == 1)
                            dstmat.SetMatrix(name, prop.matrixValue);
                        else
                            dstmat.SetMatrixArray(name, prop.matrixArray);
                        break;
                    case MaterialPropertyData.Type.Texture:
                        {
                            var rec = prop.textureValue;
                            var tex = FindTexture(rec.id);
                            if (tex != null)
                            {
                                dstmat.SetTexture(name, tex);
                                if (rec.hasScaleOffset)
                                {
                                    dstmat.SetTextureScale(name, rec.scale);
                                    dstmat.SetTextureOffset(name, rec.offset);
                                }
                            }
                        }
                        break;
                    default: break;
                }
            }

            if (onUpdateMaterial != null)
                onUpdateMaterial.Invoke(dstmat, src);
        }

        SkinnedMeshRenderer UpdateMesh(MeshData data)
        {
            var trans = UpdateTransform(data.transform);
            if (trans == null || !m_syncMeshes)
                return null;

            var data_trans = data.transform;
            var data_id = data_trans.id;
            var path = data_trans.path;

            EntityRecord rec = null;
            if (!m_clientObjects.TryGetValue(path, out rec) && data_id != Misc.InvalidID)
                m_hostObjects.TryGetValue(data_id, out rec);
            if (rec == null)
            {
                if (m_logging)
                    Debug.LogError("Something is wrong");
                return null;
            }
            else if(rec.reference != null)
            {
                // update later on UpdateReference()
                return null;
            }

            var target = rec.go.GetComponent<Transform>();
            var go = target.gameObject;

            bool activeInHierarchy = go.activeInHierarchy;
            if (!activeInHierarchy && !data.flags.hasPoints)
                return null;


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
                    // make split mesh
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

            int numSplits = Math.Max(1, data.numSplits);
            for (int si = numSplits; ; ++si)
            {
                bool created = false;
                var t = FindOrCreateObjectByPath(path + "/[" + si + "]", false, ref created);
                if (t == null)
                    break;
                DestroyImmediate(t.gameObject);
            }

            // assign materials if needed
            if (materialsUpdated)
                AssignMaterials(rec);

            return trans.GetComponent<SkinnedMeshRenderer>();
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

        PointCache UpdatePoints(PointsData data)
        {
            var trans = UpdateTransform(data.transform);
            if (trans == null || !m_syncPoints)
                return null;

            // note: reference will be resolved in UpdateReference()

            var data_trans = data.transform;
            var data_id = data_trans.id;
            var path = data_trans.path;
            var go = trans.gameObject;

            Misc.GetOrAddComponent<PointCacheRenderer>(go);
            var pts = Misc.GetOrAddComponent<PointCache>(go);

            int numData = data.numData;
            if (numData == 1)
            {
                ReadPointsData(data.GetData(0), pts.current);
            }
            else
            {
                var dst = pts.data;
                Misc.Resize(dst, numData);
                for (int di = 0; di < numData; ++di)
                    ReadPointsData(data.GetData(di), dst[di]);
            }
            return pts;
        }

        void ReadPointsData(PointsCacheData src, PointCache.Data dst)
        {
            dst.Clear();

            var flags = src.flags;
            var time = src.time;
            var num = src.numPoints;

            if (time >= 0.0f)
                dst.time = time;
            dst.bounds = src.bounds;
            if (flags.hasPoints)
            {
                dst.points = new Vector3[num];
                src.ReadPoints(dst.points);
            }
            if (flags.hasRotations)
            {
                dst.rotations = new Quaternion[num];
                src.ReadRotations(dst.rotations);
            }
            if (flags.hasScales)
            {
                dst.scales = new Vector3[num];
                src.ReadScales(dst.scales);
            }
        }

        Transform UpdateTransform(TransformData data)
        {
            var path = data.path;
            int data_id = data.id;
            if(path.Length == 0) { return null; }

            Transform trans = null;
            EntityRecord rec = null;
            if (data_id != Misc.InvalidID)
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
            if (rec == null)
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
                rec = new EntityRecord
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

            var cam = Misc.GetOrAddComponent<Camera>(trans.gameObject);
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

            var lt = Misc.GetOrAddComponent<Light>(trans.gameObject);
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
                var dstpr = dstgo.GetComponent<PointCacheRenderer>();
                if (dstpr != null)
                {
                    dstpr.sharedMesh = srcsmr.sharedMesh;

                    var materials = srcsmr.sharedMaterials;
                    for (int i = 0; i < materials.Length; ++i)
                        materials[i].enableInstancing = true;
                    dstpr.sharedMaterials = materials;
                }
                else
                {
                    var dstsmr = Misc.GetOrAddComponent<SkinnedMeshRenderer>(dstgo);
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
                        var c = Misc.GetOrAddComponent<AimConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Parent:
                    {
                        var c = Misc.GetOrAddComponent<ParentConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Position:
                    {
                        var c = Misc.GetOrAddComponent<PositionConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Rotation:
                    {
                        var c = Misc.GetOrAddComponent<RotationConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Scale:
                    {
                        var c = Misc.GetOrAddComponent<ScaleConstraint>(trans.gameObject);
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
            InterpolationMethod interpolation = AnimationData.SmoothInterpolation;
            switch (m_animtionInterpolation)
            {
                case InterpolationType.Linear:
                    interpolation = AnimationData.LinearInterpolation;
                    break;
                case InterpolationType.Constant:
                    interpolation = AnimationData.ConstantInterpolation;
                    break;
                default:
                    break;
            }

            var animClipCache = new Dictionary<GameObject, AnimationClip>();

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

                Animator animator = Misc.GetOrAddComponent<Animator>(root.gameObject);

                // get or create animation clip
                AnimationClip clip = null;
                if (!animClipCache.TryGetValue(root.gameObject, out clip))
                {
                    if (animator.runtimeAnimatorController != null)
                    {
                        var clips = animator.runtimeAnimatorController.animationClips;
                        if (clips != null && clips.Length > 0)
                        {
                            // note: this is extremely slow. animClipCache exists to cache the result and avoid frequent call.
                            var tmp = animator.runtimeAnimatorController.animationClips[0];
                            if (tmp != null)
                            {
                                clip = tmp;
                                animClipCache[root.gameObject] = tmp;
                            }
                        }
                    }
                    if (clip == null)
                    {
                        clip = new AnimationClip();
                        var clipName = clipData.name;
                        if (clipName.Length > 0)
                            clipName = root.name + "_" + clipName;
                        else
                            clipName = root.name;

                        var dstPath = assetPath + "/" + Misc.SanitizeFileName(clipName) + ".anim";
                        CreateAsset(clip, dstPath);
                        animator.runtimeAnimatorController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPathWithClip(dstPath + ".controller", clip);
                        animClipCache[root.gameObject] = clip;
                    }
                }

                var animPath = path.Replace("/" + root.name, "");
                if (animPath.Length > 0 && animPath[0] == '/')
                    animPath = animPath.Remove(0, 1);

                // get animation curves
                data.ExportToClip(clip, root.gameObject, target.gameObject, animPath, interpolation);
            }

            // smooth rotation curves
            if (m_animtionInterpolation == InterpolationType.Smooth)
                foreach (var kvp in animClipCache)
                    kvp.Value.EnsureQuaternionContinuity();

            // fire event
            if (onUpdateAnimation != null)
                foreach (var kvp in animClipCache)
                    onUpdateAnimation.Invoke(kvp.Value, clipData);
#endif
        }

        public void ReassignMaterials()
        {
            foreach (var rec in m_clientObjects)
                AssignMaterials(rec.Value);
            foreach (var rec in m_hostObjects)
                AssignMaterials(rec.Value);
        }

        void AssignMaterials(EntityRecord rec)
        {
            if (rec.go == null)
                return;

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
                    if (t == null)
                        break;
                    r = t.GetComponent<Renderer>();
                }
                if (r == null)
                    continue;

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
        #endregion


        #region ServeScene
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
                    m_hostObjects[dst_trans.id] = new EntityRecord();
                }
                var rec = m_hostObjects[dst_trans.id];
                rec.go = renderer.gameObject;
                rec.origMesh = origMesh;

                dst_trans.path = BuildPath(renderer.GetComponent<Transform>());
                m_server.ServeMesh(dst);
            }
            return ret;
        }
        bool ServeTexture(Texture2D v, GetMessage mes)
        {
            var data = TextureData.Create();
            data.name = v.name;
            // todo
            m_server.ServeTexture(data);
            return true;
        }
        bool ServeMaterial(Material mat, GetMessage mes)
        {
            var data = MaterialData.Create();
            data.name = mat.name;
            if (mat.HasProperty("_Color"))
                data.color = mat.GetColor("_Color");
            m_server.ServeMaterial(data);
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
            if (cloth != null && mes.bakeCloth)
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
                if (!flags.getMaterialIDs || materials == null || materials.Length == 0)
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
        #endregion


        #region Tools
#if UNITY_EDITOR
        public void GenerateLightmapUV(GameObject go)
        {
            if (go == null)
                return;

            var smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null)
            {
                var mesh = smr.sharedMesh;
                if (mesh != null)
                {
                    var uv = new List<Vector2>();
                    mesh.GetUVs(0, uv);
                    if (uv.Count == 0)
                    {
                        Unwrapping.GenerateSecondaryUVSet(mesh);
                        Debug.Log("generated uv " + mesh.name);
                    }
                }
            }
            for (int i = 1; ; ++i)
            {
                var t = go.transform.Find("[" + i + "]");
                if (t == null)
                    break;
                GenerateLightmapUV(t.gameObject);
            }
        }
        public void GenerateLightmapUV()
        {
            foreach (var kvp in m_clientObjects)
                GenerateLightmapUV(kvp.Value.go);
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

            MakeSureAssetDirectoryExists();
            foreach (var m in m_materialList)
            {
                var material = m.material;
                if (AssetDatabase.GetAssetPath(material) == "")
                {
                    string dstPath = assetPath + "/" + material.name + ".mat";
                    CreateAsset(material, dstPath);
                    if (m_logging)
                        Debug.Log("exported material " + dstPath);
                }
            }
        }

        void ExportMeshes(GameObject go)
        {
            if(go == null)
                return;

            var smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr == null)
                return;

            var mesh = smr.sharedMesh;
            if (mesh == null || AssetDatabase.GetAssetPath(mesh) != "")
                return;

            var dstPath = assetPath + "/" + mesh.name + ".asset";
            CreateAsset(mesh, dstPath);
            if (m_logging)
                Debug.Log("exported mesh " + dstPath);

            // export splits
            for (int i = 1; ; ++i)
            {
                var t = go.transform.Find("[" + i + "]");
                if (t == null)
                    break;
                ExportMeshes(t.gameObject);
            }
        }
        public void ExportMeshes()
        {
            MakeSureAssetDirectoryExists();

            // export client meshes
            foreach (var kvp in m_clientObjects)
            {
                if(kvp.Value.go == null || !kvp.Value.go.activeInHierarchy) { continue; }
                if (kvp.Value.editMesh != null)
                    ExportMeshes(kvp.Value.go);
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
                    if (mf != null)
                        mf.sharedMesh = kvp.Value.origMesh;

                    var smr = kvp.Value.go.GetComponent<SkinnedMeshRenderer>();
                    if (smr != null)
                        smr.sharedMesh = kvp.Value.origMesh;

                    if (m_logging)
                        Debug.Log("updated mesh " + kvp.Value.origMesh.name);
                    ++n;
                }
            }
            if (n > 0)
                AssetDatabase.SaveAssets();
        }

        void CheckMaterialAssignedViaEditor()
        {
            bool changed = false;
            foreach(var kvp in m_clientObjects)
            {
                var rec = kvp.Value;
                if (rec.go != null && rec.go.activeInHierarchy)
                {
                    var mr = rec.go.GetComponent<SkinnedMeshRenderer>();
                    if (mr == null || rec.submeshCounts.Length == 0)
                        continue;

                    var materials = mr.sharedMaterials;
                    int n = Math.Min(materials.Length, rec.submeshCounts[0]);
                    for (int i = 0; i < n; ++i)
                    {
                        int mid = rec.materialIDs[i];
                        var mrec = m_materialList.Find(a => a.id == mid);
                        if (mrec == null)
                            continue;

                        if (materials[i] != mrec.material)
                        {
                            mrec.material = materials[i];
                            changed = true;
                            break;
                        }
                    }
                }
                if (changed)
                    break;
            }

            if (changed)
            {
                ReassignMaterials();
                ForceRepaint();
            }
        }

        void OnSceneViewGUI(SceneView sceneView)
        {
            if (m_trackMaterialAssignment)
            {
                if (Event.current.type == EventType.DragExited && Event.current.button == 0)
                    CheckMaterialAssignedViaEditor();
            }
        }
#endif
        #endregion


        #region EventFunctions
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
#if UNITY_EDITOR
            DeployStreamingAssets.Deploy();
#endif
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
        #endregion
    }
}
