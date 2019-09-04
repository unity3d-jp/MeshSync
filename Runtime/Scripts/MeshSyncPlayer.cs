using System;
using System.Linq;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEngine.SceneManagement;
#if UNITY_2017_1_OR_NEWER
using UnityEngine.Animations;
#endif
#if UNITY_2019_1_OR_NEWER
using Unity.Collections;
#endif

#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.SceneManagement;
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
    public class MeshSyncPlayer : MonoBehaviour, ISerializationCallbackReceiver
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
            public EntityType dataType;
            public int index;
            public GameObject go;
            public Transform trans;
            public Camera camera;
            public Light light;
            public MeshFilter meshFilter;
            public MeshRenderer meshRenderer;
            public SkinnedMeshRenderer skinnedMeshRenderer;
            public PointCache pointCache;
            public PointCacheRenderer pointCacheRenderer;
            public Mesh origMesh;
            public Mesh mesh;

            public int[] materialIDs;
            public string reference;
            public string rootBonePath;
            public string[] bonePaths;
            public bool smrUpdated = false;
            public bool smrEnabled = false;
            public bool hasVisibility = false;
            public VisibilityFlags visibility; // for reference
            public bool recved = false;

            // return true if modified
            public bool BuildMaterialData(MeshData md)
            {
                int numSubmeshes = md.numSubmeshes;

                bool updated = false;
                if (materialIDs == null || materialIDs.Length != numSubmeshes)
                {
                    materialIDs = new int[numSubmeshes];
                    updated = true;
                }
                for (int i = 0; i < numSubmeshes; ++i)
                {
                    int mid = md.GetSubmesh(i).materialID;
                    if (!updated)
                        updated = materialIDs[i] != mid;
                    materialIDs[i] = mid;
                }
                return updated;
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
        #endregion


        #region Fields
        [SerializeField] protected DataPath m_assetDir = new DataPath(DataPath.Root.DataPath, "MeshSyncAssets");
        [SerializeField] protected Transform m_rootObject;

        [SerializeField] protected bool m_syncVisibility = true;
        [SerializeField] protected bool m_syncTransform = true;
        [SerializeField] protected bool m_syncCameras = true;
        [SerializeField] protected bool m_syncLights = true;
        [SerializeField] protected bool m_syncMeshes = true;
        [SerializeField] protected bool m_syncPoints = true;
        [SerializeField] protected bool m_syncMaterials = true;
        [SerializeField] protected bool m_handleAssets = true;

#if UNITY_2018_1_OR_NEWER
        [SerializeField] protected bool m_usePhysicalCameraParams = true;
#endif
        [SerializeField] protected bool m_useCustomCameraMatrices = true;
        [SerializeField] protected bool m_updateMeshColliders = true;
        [SerializeField] protected bool m_findMaterialFromAssets = true;
        [SerializeField] protected InterpolationMode m_animationInterpolation = InterpolationMode.Smooth;
        [SerializeField] protected bool m_keyframeReduction = true;
        [SerializeField] protected float m_reductionThreshold = 0.001f;
        [SerializeField] protected bool m_reductionEraseFlatCurves = false;
        [SerializeField] protected ZUpCorrectionMode m_zUpCorrection = ZUpCorrectionMode.FlipYZ;
        [SerializeField] protected bool m_logging = true;
        [SerializeField] protected bool m_profiling = false;
        [SerializeField] protected bool m_markMeshesDynamic = false;
        [SerializeField] protected bool m_dontSaveAssetsInScene = false;

        [SerializeField] protected Material m_dummyMaterial;
        [SerializeField] protected List<MaterialHolder> m_materialList = new List<MaterialHolder>();
        [SerializeField] protected List<TextureHolder> m_textureList = new List<TextureHolder>();
        [SerializeField] protected List<AudioHolder> m_audioList = new List<AudioHolder>();

        [SerializeField] string[] m_clientObjects_keys;
        [SerializeField] EntityRecord[] m_clientObjects_values;
        [SerializeField] int[] m_hostObjects_keys;
        [SerializeField] EntityRecord[] m_hostObjects_values;
        [SerializeField] GameObject[] m_objIDTable_keys;
        [SerializeField] int[] m_objIDTable_values;
        [SerializeField] int m_objIDSeed = 0;

#if UNITY_EDITOR
        [SerializeField] bool m_syncMaterialList = true;
        [SerializeField] bool m_sortEntities = true;
        [SerializeField] bool m_progressiveDisplay = true;
        [SerializeField] bool m_foldSyncSettings = true;
        [SerializeField] bool m_foldImportSettings = true;
        [SerializeField] bool m_foldMisc = true;
        [SerializeField] bool m_foldMaterialList = true;
        [SerializeField] bool m_foldAnimationTweak = true;
        [SerializeField] bool m_foldExportAssets = true;
        bool m_recordAssignMaterials = false;
#endif

        protected bool m_needReassignMaterials = false;

        protected Dictionary<string, EntityRecord> m_clientObjects = new Dictionary<string, EntityRecord>();
        protected Dictionary<int, EntityRecord> m_hostObjects = new Dictionary<int, EntityRecord>();
        protected Dictionary<GameObject, int> m_objIDTable = new Dictionary<GameObject, int>();
        #endregion


        #region Properties
        public static int pluginVersion { get { return Lib.pluginVersion; } }
        public static int protocolVersion { get { return Lib.protocolVersion; } }
        public DataPath assetDir
        {
            get { return m_assetDir; }
            set { m_assetDir = value; }
        }
        public string assetPath
        {
            get { return m_assetDir.leaf.Length != 0 ? "Assets/" + m_assetDir.leaf : "Assets"; }
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

        public bool syncVisibility
        {
            get { return m_syncVisibility; }
            set { m_syncVisibility = value; }
        }
        public bool syncTransform
        {
            get { return m_syncTransform; }
            set { m_syncTransform = value; }
        }
        public bool syncCameras
        {
            get { return m_syncCameras; }
            set { m_syncCameras = value; }
        }
        public bool syncLights
        {
            get { return m_syncLights; }
            set { m_syncLights = value; }
        }
        public bool syncMeshes
        {
            get { return m_syncMeshes; }
            set { m_syncMeshes = value; }
        }
        public bool syncPoints
        {
            get { return m_syncPoints; }
            set { m_syncPoints = value; }
        }
        public bool syncMaterials
        {
            get { return m_syncMaterials; }
            set { m_syncMaterials = value; }
        }
        public bool handleAssets
        {
            get { return m_handleAssets; }
            set { m_handleAssets = value; }
        }

        public InterpolationMode animationInterpolation
        {
            get { return m_animationInterpolation; }
            set { m_animationInterpolation = value; }
        }
        public bool keyframeReduction
        {
            get { return m_keyframeReduction; }
            set { m_keyframeReduction = value; }
        }
        public float reductionThreshold
        {
            get { return m_reductionThreshold; }
            set { m_reductionThreshold = value; }
        }
        public bool reductionKeepEraseCurves
        {
            get { return m_reductionEraseFlatCurves; }
            set { m_reductionEraseFlatCurves = value; }
        }
        public ZUpCorrectionMode zUpCorrection
        {
            get { return m_zUpCorrection; }
            set { m_zUpCorrection = value; }
        }

#if UNITY_2018_1_OR_NEWER
        public bool usePhysicalCameraParams
        {
            get { return m_usePhysicalCameraParams; }
            set { m_usePhysicalCameraParams = value; }
        }
#endif
        public bool useCustomCameraMatrices
        {
            get { return m_useCustomCameraMatrices; }
            set { m_useCustomCameraMatrices = value; }
        }
        public bool updateMeshColliders
        {
            get { return m_updateMeshColliders; }
            set { m_updateMeshColliders = value; }
        }
        public bool findMaterialFromAssets
        {
            get { return m_findMaterialFromAssets; }
            set { m_findMaterialFromAssets = value; }
        }

        public bool logging
        {
            get { return m_logging; }
            set { m_logging = value; }
        }
        public bool profiling
        {
            get { return m_profiling; }
            set { m_profiling = value; }
        }

        public bool markMeshesDynamic
        {
            get { return m_markMeshesDynamic; }
            set { m_markMeshesDynamic = value; }
        }
        public bool dontSaveAssetsInScene
        {
            get { return m_dontSaveAssetsInScene; }
            set { m_dontSaveAssetsInScene = value; }
        }

        public List<MaterialHolder> materialList { get { return m_materialList; } }
        public List<TextureHolder> textureList { get { return m_textureList; } }

#if UNITY_EDITOR
        public bool syncMaterialList
        {
            get { return m_syncMaterialList; }
            set { m_syncMaterialList = value; }
        }
        public bool sortEntities
        {
            get { return m_sortEntities; }
            set { m_sortEntities = value; }
        }
        public bool progressiveDisplay
        {
            get { return m_progressiveDisplay; }
            set { m_progressiveDisplay = value; }
        }

        public bool foldSyncSettings
        {
            get { return m_foldSyncSettings; }
            set { m_foldSyncSettings = value; }
        }
        public bool foldImportSettings
        {
            get { return m_foldImportSettings; }
            set { m_foldImportSettings = value; }
        }
        public bool foldMisc
        {
            get { return m_foldMisc; }
            set { m_foldMisc = value; }
        }
        public bool foldMaterialList
        {
            get { return m_foldMaterialList; }
            set { m_foldMaterialList = value; }
        }
        public bool foldAnimationTweak
        {
            get { return m_foldAnimationTweak; }
            set { m_foldAnimationTweak = value; }
        }
        public bool foldExportAssets
        {
            get { return m_foldExportAssets; }
            set { m_foldExportAssets = value; }
        }
#endif
        #endregion


        #region Impl
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
        #endregion

        #region Misc
        //[MethodImpl(MethodImplOptions.AggressiveInlining)]
        protected bool Try(Action act)
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
            m_assetDir.CreateDirectory();
#endif
        }

        // this function has a complex behavior to keep existing .meta:
        //  if an asset already exists in assetPath, load it and copy the content of obj to it and replace obj with it.
        //  otherwise obj is simply saved by AssetDatabase.CreateAsset().
        protected bool SaveAsset<T>(ref T obj, string assetPath) where T : UnityEngine.Object
        {
#if UNITY_EDITOR
            var ret = Misc.SaveAsset(obj, assetPath);
            if (ret != null)
            {
                obj = ret;
                return true;
            }
#endif
            return false;
        }

        protected bool IsAsset(UnityEngine.Object obj)
        {
#if UNITY_EDITOR
            return AssetDatabase.Contains(obj);
#else
            return false;
#endif
        }

        protected bool DestroyIfNotAsset(UnityEngine.Object obj)
        {
            if (obj != null && IsAsset(obj))
            {
                DestroyImmediate(obj, false);
                return true;
            }
            return false;
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
            if (id == Lib.invalidID)
                return null;
            var rec = m_textureList.Find(a => a.id == id);
            return rec != null ? rec.texture : null;
        }

        public Material FindMaterial(int id)
        {
            if (id == Lib.invalidID)
                return null;
            var rec = m_materialList.Find(a => a.id == id);
            return rec != null ? rec.material : null;
        }

        public bool EraseMaterialRecord(int id)
        {
            return m_materialList.RemoveAll(v => v.id == id) != 0;
        }

        protected int GetMaterialIndex(Material mat)
        {
            if (mat == null)
                return Lib.invalidID;

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
            if (id == Lib.invalidID)
                return null;
            var rec = m_audioList.Find(a => a.id == id);
            return rec != null ? rec.audio : null;
        }

        public int GetObjectlID(GameObject go)
        {
            if (go == null)
                return Lib.invalidID;

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

        public Transform FindOrCreateObjectByPath(string path, bool createIfNotExist, ref bool created)
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

        public Transform FindOrCreateObjectByName(Transform parent, string name, bool createIfNotExist, ref bool created)
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
            // prefer non Standard shader because it will be pink in HDRP
            var shader = Shader.Find("HDRP/Lit");
            if (shader == null)
                shader = Shader.Find("LWRP/Lit");
            if (shader == null)
                shader = Shader.Find("Standard");
            var ret = new Material(shader);
            return ret;
        }

        public void ForceRepaint()
        {
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying && !EditorApplication.isPaused)
            {
                SceneView.RepaintAll();
                UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
            }
#endif
        }
        #endregion

        #region ReceiveScene
        public void BeforeUpdateScene()
        {
            MakeSureAssetDirectoryExists();

            if (onSceneUpdateBegin != null)
                onSceneUpdateBegin.Invoke();
        }

        public void UpdateScene(SceneData scene)
        {
            // handle assets
            Try(() =>
            {
                int numAssets = scene.numAssets;
                if (numAssets > 0)
                {
                    bool save = false;
                    for (int i = 0; i < numAssets; ++i)
                    {
                        var asset = scene.GetAsset(i);
                        switch (asset.type)
                        {
                            case AssetType.File:
                                UpdateFileAsset((FileAssetData)asset);
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
                                save = true;
                                break;
                            default:
                                if (m_logging)
                                    Debug.Log("unknown asset: " + asset.name);
                                break;
                        }
                    }
#if UNITY_EDITOR
                    if (save)
                        AssetDatabase.SaveAssets();
#endif
                }
            });

            // handle entities
            Try(() =>
            {
                int numObjects = scene.numEntities;
                for (int i = 0; i < numObjects; ++i)
                {
                    EntityRecord dst = null;
                    var src = scene.GetEntity(i);
                    switch (src.entityType)
                    {
                        case EntityType.Transform:
                            dst = UpdateTransform(src);
                            break;
                        case EntityType.Camera:
                            dst = UpdateCamera((CameraData)src);
                            break;
                        case EntityType.Light:
                            dst = UpdateLight((LightData)src);
                            break;
                        case EntityType.Mesh:
                            dst = UpdateMesh((MeshData)src);
                            break;
                        case EntityType.Points:
                            dst = UpdatePoints((PointsData)src);
                            break;
                    }

                    if (dst != null && onUpdateEntity != null)
                        onUpdateEntity.Invoke(dst.go, src);
                }
            });

#if UNITY_2018_1_OR_NEWER
            // handle constraints
            Try(() =>
            {
                int numConstraints = scene.numConstraints;
                for (int i = 0; i < numConstraints; ++i)
                    UpdateConstraint(scene.GetConstraint(i));
            });
#endif
#if UNITY_EDITOR
            if (m_progressiveDisplay)
                ForceRepaint();
#endif
        }

        public void AfterUpdateScene()
        {
            List<string> deadKeys = null;

            // resolve bones
            foreach (var kvp in m_clientObjects)
            {
                var rec = kvp.Value;
                if (rec.go == null)
                {
                    if (deadKeys == null)
                        deadKeys = new List<string>();
                    deadKeys.Add(kvp.Key);
                    continue;
                }

                if (rec.smrUpdated)
                {
                    rec.smrUpdated = false;

                    var smr = rec.skinnedMeshRenderer;
                    if (rec.bonePaths != null && rec.bonePaths.Length > 0)
                    {
                        var boneCount = rec.bonePaths.Length;
                        bool dummy = false;

                        var bones = new Transform[boneCount];
                        for (int bi = 0; bi < boneCount; ++bi)
                            bones[bi] = FindOrCreateObjectByPath(rec.bonePaths[bi], false, ref dummy);

                        Transform root = null;
                        if (!string.IsNullOrEmpty(rec.rootBonePath))
                            root = FindOrCreateObjectByPath(rec.rootBonePath, false, ref dummy);
                        if (root == null && boneCount > 0)
                        {
                            // find root bone
                            root = bones[0];
                            for (; ; )
                            {
                                var parent = root.parent;
                                if (parent == null || parent == m_rootObject)
                                    break;
                                root = parent;
                            }
                        }

                        smr.rootBone = root;
                        smr.bones = bones;
                        smr.updateWhenOffscreen = true; // todo: this should be turned off at some point

                        rec.bonePaths = null;
                        rec.rootBonePath = null;
                    }
                    smr.enabled = rec.smrEnabled;
                }
            }
            if (deadKeys != null)
                foreach (var key in deadKeys)
                    m_clientObjects.Remove(key);

            // resolve references
            // this must be another pass because resolving bones can affect references
            foreach (var kvp in m_clientObjects)
            {
                var rec = kvp.Value;
                if (!string.IsNullOrEmpty(rec.reference))
                {
                    EntityRecord srcrec = null;
                    if (m_clientObjects.TryGetValue(rec.reference, out srcrec) && srcrec.go != null)
                    {
                        rec.materialIDs = srcrec.materialIDs;
                        UpdateReference(rec, srcrec);
                    }
                }
            }

            // reassign materials
            if (m_needReassignMaterials)
            {
                m_materialList = m_materialList.OrderBy(v => v.index).ToList();
                ReassignMaterials();
                m_needReassignMaterials = false;
            }

#if UNITY_EDITOR
            // sort objects by index
            if (m_sortEntities)
            {
                var rec = m_clientObjects.Values.OrderBy(v => v.index);
                foreach (var r in rec)
                {
                    if (r.go != null)
                        r.go.GetComponent<Transform>().SetSiblingIndex(r.index + 1000);
                }
            }

            if (!EditorApplication.isPlaying || !EditorApplication.isPaused)
            {
                // force recalculate skinning
                foreach (var kvp in m_clientObjects)
                {
                    var rec = kvp.Value;
                    var smr = rec.skinnedMeshRenderer;
                    if (smr != null && rec.smrEnabled && rec.go.activeInHierarchy)
                    {
                        smr.enabled = false; // 
                        smr.enabled = true;  // force recalculate skinned mesh on editor. I couldn't find better way...
                    }
                }
            }

            if (!EditorApplication.isPlaying)
            {
                // mark scene dirty
                EditorSceneManager.MarkSceneDirty(SceneManager.GetActiveScene());
            }
#endif

            if (onSceneUpdateEnd != null)
                onSceneUpdateEnd.Invoke();
        }

        void UpdateFileAsset(FileAssetData src)
        {
#if UNITY_EDITOR
            if (!m_handleAssets)
                return;

            src.WriteToFile(assetPath + "/" + src.name);
#endif
        }

        void UpdateAudio(AudioData src)
        {
            if (!m_handleAssets)
                return;

            AudioClip ac = null;

            var format = src.format;
            if (format == AudioFormat.RawFile)
            {
#if UNITY_EDITOR
                // create file and import it
                var dstPath = assetPath + "/" + src.name;
                src.WriteToFile(dstPath);
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
                // export as .wav and import it
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
            if (!m_handleAssets)
                return;

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
            var materialID = src.id;
            var materialName = src.name;

            var dst = m_materialList.Find(a => a.id == materialID);
            if (dst == null)
            {
                dst = new MaterialHolder();
                dst.id = materialID;
                m_materialList.Add(dst);
            }
#if UNITY_EDITOR
            if (m_findMaterialFromAssets && m_syncMaterials && (dst.material == null || dst.name != materialName))
            {
                Material candidate = null;

                var guids = AssetDatabase.FindAssets("t:Material " + materialName);
                foreach (var guid in guids)
                {
                    var path = AssetDatabase.GUIDToAssetPath(guid);
                    var material = AssetDatabase.LoadAssetAtPath<Material>(path);
                    if (material.name == materialName)
                    {
                        candidate = material;

                        // if there are multiple candidates, prefer the editable one (= not a part of fbx etc)
                        if (((int)material.hideFlags & (int)HideFlags.NotEditable) == 0)
                            break;
                    }
                }

                if (candidate != null)
                {
                    dst.material = candidate;
                    dst.materialIID = 0; // ignore material params
                    m_needReassignMaterials = true;
                }
            }
#endif
            if (dst.material == null)
            {
                // prefer non Standard shader because it will be pink in HDRP
                var shaderName = src.shader;
                if (shaderName != "Standard")
                {
                    var shader = Shader.Find(src.shader);
                    if (shader != null)
                        dst.material = new Material(shader);
                }
                if (dst.material == null)
                    dst.material = CreateDefaultMaterial();
                dst.material.name = materialName;

                dst.materialIID = dst.material.GetInstanceID();
                m_needReassignMaterials = true;
            }
            dst.name = materialName;
            dst.index = src.index;
            dst.shader = src.shader;
            dst.color = src.color;

            var dstmat = dst.material;
            if (m_syncMaterials && dst.materialIID == dst.material.GetInstanceID())
            {
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
                    var propName = prop.name;
                    var propType = prop.type;
                    if (!dstmat.HasProperty(propName))
                        continue;

                    // todo: handle transparent
                    //if (propName == _Color)
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
                    if (propName == _EmissionColor)
                    {
                        if (dstmat.globalIlluminationFlags == MaterialGlobalIlluminationFlags.EmissiveIsBlack)
                        {
                            dstmat.globalIlluminationFlags = MaterialGlobalIlluminationFlags.RealtimeEmissive;
                            dstmat.EnableKeyword(_EMISSION);
                        }
                    }
                    else if (propName == _MetallicGlossMap)
                    {
                        dstmat.EnableKeyword(_METALLICGLOSSMAP);
                    }
                    else if (propName == _BumpMap)
                    {
                        dstmat.EnableKeyword(_NORMALMAP);
                    }

                    int len = prop.arrayLength;
                    switch (propType)
                    {
                        case MaterialPropertyData.Type.Int:
                            dstmat.SetInt(propName, prop.intValue);
                            break;
                        case MaterialPropertyData.Type.Float:
                            if (len == 1)
                                dstmat.SetFloat(propName, prop.floatValue);
                            else
                                dstmat.SetFloatArray(propName, prop.floatArray);
                            break;
                        case MaterialPropertyData.Type.Vector:
                            if (len == 1)
                                dstmat.SetVector(propName, prop.vectorValue);
                            else
                                dstmat.SetVectorArray(propName, prop.vectorArray);
                            break;
                        case MaterialPropertyData.Type.Matrix:
                            if (len == 1)
                                dstmat.SetMatrix(propName, prop.matrixValue);
                            else
                                dstmat.SetMatrixArray(propName, prop.matrixArray);
                            break;
                        case MaterialPropertyData.Type.Texture:
                            {
                                var rec = prop.textureValue;
                                var tex = FindTexture(rec.id);
                                if (tex != null)
                                {
                                    dstmat.SetTexture(propName, tex);
                                    if (rec.hasScaleOffset)
                                    {
                                        dstmat.SetTextureScale(propName, rec.scale);
                                        dstmat.SetTextureOffset(propName, rec.offset);
                                    }
                                }
                            }
                            break;
                        default: break;
                    }
                }
            }

            if (onUpdateMaterial != null)
                onUpdateMaterial.Invoke(dstmat, src);
        }

        EntityRecord UpdateMesh(MeshData data)
        {
            if (!m_syncMeshes)
                return null;

            var dtrans = data.transform;
            var dflags = data.dataFlags;
            var rec = UpdateTransform(dtrans);
            if (rec == null || dflags.unchanged)
                return null;

            if (!string.IsNullOrEmpty(rec.reference))
            {
                // references will be resolved later in UpdateReference()
                return null;
            }

            var path = dtrans.path;
            var go = rec.go;
            var trans = go.transform;
            bool activeInHierarchy = go.activeInHierarchy;
            if (!activeInHierarchy && !dflags.hasPoints)
                return null;


            // allocate material list
            bool materialsUpdated = rec.BuildMaterialData(data);
            bool meshUpdated = false;

            if (dflags.hasPoints && dflags.hasIndices)
            {
                // note:
                // assume there is always only 1 mesh split.
                // old versions supported multiple splits because vertex index was 16 bit (pre-Unity 2017.3),
                // but that code path was removed for simplicity and my sanity.
                if (data.numIndices == 0)
                {
                    if (rec.mesh != null)
                        rec.mesh.Clear();
                }
                else
                {
                    if (rec.mesh == null)
                    {
                        rec.mesh = new Mesh();
                        rec.mesh.name = trans.name;
                        if (m_markMeshesDynamic)
                            rec.mesh.MarkDynamic();
                        if (m_dontSaveAssetsInScene)
                            rec.mesh.hideFlags = HideFlags.DontSaveInEditor;
#if UNITY_2017_3_OR_NEWER
                        rec.mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
#endif
                    }
                    UpdateMesh(ref rec.mesh, data);
                }
                meshUpdated = true;
            }

            if (dflags.hasBones || dflags.hasBlendshapes)
            {
                var smr = rec.skinnedMeshRenderer;
                if (smr == null)
                {
                    materialsUpdated = true;
                    smr = rec.skinnedMeshRenderer = Misc.GetOrAddComponent<SkinnedMeshRenderer>(trans.gameObject);
                    if (rec.meshRenderer != null)
                    {
                        DestroyImmediate(rec.meshRenderer);
                        rec.meshRenderer = null;
                    }
                    if (rec.meshFilter != null)
                    {
                        DestroyImmediate(rec.meshFilter);
                        rec.meshFilter = null;
                    }
                }

                rec.smrUpdated = true;
                if (m_syncVisibility && dtrans.dataFlags.hasVisibility)
                    rec.smrEnabled = data.transform.visibility.visibleInRender;
                else
                    rec.smrEnabled = smr.enabled;
                // disable temporarily to prevent error. restore on AfterUpdateScene()
                smr.enabled = false;
                smr.sharedMesh = rec.mesh;

                // update bones
                if (dflags.hasBones)
                {
                    if (dflags.hasRootBone)
                        rec.rootBonePath = data.rootBonePath;
                    rec.bonePaths = data.bonePaths;
                    // bones will be resolved in AfterUpdateScene()
                }
                else
                {
                    smr.localBounds = rec.mesh.bounds;
                    smr.updateWhenOffscreen = false;
                }

                // update blendshape weights
                if (dflags.hasBlendshapes)
                {
                    int numBlendShapes = Math.Min(data.numBlendShapes, rec.mesh.blendShapeCount);
                    for (int bi = 0; bi < numBlendShapes; ++bi)
                    {
                        var bsd = data.GetBlendShapeData(bi);
                        smr.SetBlendShapeWeight(bi, bsd.weight);
                    }
                }
            }
            else if (meshUpdated)
            {
                var mf = rec.meshFilter;
                var mr = rec.meshRenderer;
                if (mf == null)
                {
                    materialsUpdated = true;
                    mf = rec.meshFilter = Misc.GetOrAddComponent<MeshFilter>(trans.gameObject);
                    mr = rec.meshRenderer = Misc.GetOrAddComponent<MeshRenderer>(trans.gameObject);
                    if (rec.skinnedMeshRenderer != null)
                    {
                        mr.sharedMaterials = rec.skinnedMeshRenderer.sharedMaterials;
                        DestroyImmediate(rec.skinnedMeshRenderer);
                        rec.skinnedMeshRenderer = null;
                    }
                }

                if (m_syncVisibility && dtrans.dataFlags.hasVisibility)
                    mr.enabled = data.transform.visibility.visibleInRender;
                mf.sharedMesh = rec.mesh;
                rec.smrEnabled = false;
            }

            if (meshUpdated)
            {
                var collider = m_updateMeshColliders ? trans.GetComponent<MeshCollider>() : null;
                if (collider != null &&
                    (collider.sharedMesh == null || collider.sharedMesh == rec.mesh))
                {
                    collider.sharedMesh = rec.mesh;
                }
            }

            // assign materials if needed
            if (materialsUpdated)
                AssignMaterials(rec);

            return rec;
        }

        PinnedList<int> m_tmpI = new PinnedList<int>();
        PinnedList<Vector2> m_tmpV2 = new PinnedList<Vector2>();
        PinnedList<Vector3> m_tmpV3 = new PinnedList<Vector3>();
        PinnedList<Vector4> m_tmpV4 = new PinnedList<Vector4>();
        PinnedList<Color> m_tmpC = new PinnedList<Color>();

        void UpdateMesh(ref Mesh mesh, MeshData data)
        {
            bool keepIndices = false;
            if (mesh.vertexCount != 0)
            {
                if (data.dataFlags.topologyUnchanged)
                    keepIndices = true;
                else
                {
                    mesh.Clear();
                    // Mesh.Clear() seems don't clear bindposes
                    mesh.bindposes = null;
                }
            }

            var numPoints = data.numPoints;
            var dataFlags = data.dataFlags;
            if (dataFlags.hasPoints)
            {
                m_tmpV3.Resize(numPoints);
                data.ReadPoints(m_tmpV3);
                mesh.SetVertices(m_tmpV3.List);
            }
            if (dataFlags.hasNormals)
            {
                m_tmpV3.Resize(numPoints);
                data.ReadNormals(m_tmpV3);
                mesh.SetNormals(m_tmpV3.List);
            }
            if (dataFlags.hasTangents)
            {
                m_tmpV4.Resize(numPoints);
                data.ReadTangents(m_tmpV4);
                mesh.SetTangents(m_tmpV4.List);
            }
            if (dataFlags.hasUV0)
            {
                m_tmpV2.Resize(numPoints);
                data.ReadUV0(m_tmpV2);
                mesh.SetUVs(0, m_tmpV2.List);
            }
            if (dataFlags.hasUV1)
            {
                m_tmpV2.Resize(numPoints);
                data.ReadUV1(m_tmpV2);
                mesh.SetUVs(1, m_tmpV2.List);
            }
            if (dataFlags.hasColors)
            {
                m_tmpC.Resize(numPoints);
                data.ReadColors(m_tmpC);
                mesh.SetColors(m_tmpC.List);
            }
            if (dataFlags.hasBones)
            {
                mesh.bindposes = data.bindposes;
#if UNITY_2019_1_OR_NEWER
                {
                    // bonesPerVertex + weights1
                    var bonesPerVertex = new NativeArray<byte>(numPoints, Allocator.Temp);
                    var weights1 = new NativeArray<BoneWeight1>(data.numBoneWeights, Allocator.Temp);
                    data.ReadBoneCounts(Misc.ForceGetPointer(ref bonesPerVertex));
                    data.ReadBoneWeightsV(Misc.ForceGetPointer(ref weights1));
                    mesh.SetBoneWeights(bonesPerVertex, weights1);
                    bonesPerVertex.Dispose();
                    weights1.Dispose();
                }
#else
                {
                    // weights4
                    var tmpWeights4 = new PinnedList<BoneWeight>();
                    tmpWeights4.Resize(numPoints);
                    data.ReadBoneWeights4(tmpWeights4);
                    mesh.boneWeights = tmpWeights4.Array;
                    tmpWeights4.Dispose();
                }
#endif
            }
            if (dataFlags.hasIndices && !keepIndices)
            {
                int subMeshCount = data.numSubmeshes;
                mesh.subMeshCount = subMeshCount;
                for (int smi = 0; smi < subMeshCount; ++smi)
                {
                    var submesh = data.GetSubmesh(smi);
                    var topology = submesh.topology;

                    m_tmpI.Resize(submesh.numIndices);
                    submesh.ReadIndices(data, m_tmpI);

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
            if (dataFlags.hasBlendshapes)
            {
                var tmpBSP = new PinnedList<Vector3>(numPoints);
                var tmpBSN = new PinnedList<Vector3>(numPoints);
                var tmpBST = new PinnedList<Vector3>(numPoints);

                int numBlendShapes = data.numBlendShapes;
                for (int bi = 0; bi < numBlendShapes; ++bi)
                {
                    var bsd = data.GetBlendShapeData(bi);
                    var name = bsd.name;
                    var numFrames = bsd.numFrames;
                    for (int fi = 0; fi < numFrames; ++fi)
                    {
                        bsd.ReadPoints(fi, tmpBSP);
                        bsd.ReadNormals(fi, tmpBSN);
                        bsd.ReadTangents(fi, tmpBST);
                        mesh.AddBlendShapeFrame(name, bsd.GetWeight(fi), tmpBSP.Array, tmpBSN.Array, tmpBST.Array);
                    }
                }

                tmpBSP.Dispose();
                tmpBSN.Dispose();
                tmpBST.Dispose();
            }

            mesh.bounds = data.bounds;
            mesh.UploadMeshData(false);
        }

        EntityRecord UpdatePoints(PointsData data)
        {
            if (!m_syncPoints)
                return null;

            var dtrans = data.transform;
            var dflags = data.dataFlags;
            var rec = UpdateTransform(dtrans);
            if (rec == null || dflags.unchanged)
                return null;

            // reference (source mesh) will be resolved in UpdateReference()

            var path = dtrans.path;
            var go = rec.go;

            var dst = rec.pointCache;
            if (dst == null)
            {
                rec.pointCacheRenderer = Misc.GetOrAddComponent<PointCacheRenderer>(go);
                dst = rec.pointCache = Misc.GetOrAddComponent<PointCache>(go);
            }
            dst.Clear();

            var num = data.numPoints;
            dst.bounds = data.bounds;
            if (dflags.hasPoints)
            {
                dst.points = new Vector3[num];
                data.ReadPoints(dst.points);
            }
            if (dflags.hasRotations)
            {
                dst.rotations = new Quaternion[num];
                data.ReadRotations(dst.rotations);
            }
            if (dflags.hasScales)
            {
                dst.scales = new Vector3[num];
                data.ReadScales(dst.scales);
            }
            return rec;
        }

        EntityRecord UpdateTransform(TransformData data)
        {
            var path = data.path;
            int hostID = data.hostID;
            if (path.Length == 0)
                return null;

            Transform trans = null;
            EntityRecord rec = null;
            if (hostID != Lib.invalidID)
            {
                if (m_hostObjects.TryGetValue(hostID, out rec))
                {
                    if (rec.go == null)
                    {
                        m_hostObjects.Remove(hostID);
                        rec = null;
                    }
                }
                if (rec == null)
                    return null;
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
                if (rec == null)
                {
                    bool created = false;
                    trans = FindOrCreateObjectByPath(path, true, ref created);
                    rec = new EntityRecord
                    {
                        go = trans.gameObject,
                        trans = trans,
                        recved = true,
                    };
                    m_clientObjects.Add(path, rec);
                }
            }

            trans = rec.trans;
            if (trans == null)
                trans = rec.trans = rec.go.transform;

            var dflags = data.dataFlags;
            if (!dflags.unchanged)
            {
                var visibility = data.visibility;
                rec.index = data.index;
                rec.dataType = data.entityType;

                // sync TRS
                if (m_syncTransform)
                {
                    if (dflags.hasPosition)
                        trans.localPosition = data.position;
                    if (dflags.hasRotation)
                        trans.localRotation = data.rotation;
                    if (dflags.hasScale)
                        trans.localScale = data.scale;
                }

                // visibility
                if (m_syncVisibility && dflags.hasVisibility)
                    trans.gameObject.SetActive(visibility.active);

                // visibility for reference
                rec.hasVisibility = dflags.hasVisibility;
                if (rec.hasVisibility)
                    rec.visibility = visibility;

                // reference. will be resolved in AfterUpdateScene()
                if (dflags.hasReference)
                    rec.reference = data.reference;
                else
                    rec.reference = null;
            }
            return rec;
        }

        EntityRecord UpdateCamera(CameraData data)
        {
            if (!m_syncCameras)
                return null;

            var dtrans = data.transform;
            var dflags = data.dataFlags;
            var rec = UpdateTransform(dtrans);
            if (rec == null || dflags.unchanged)
                return null;

            var cam = rec.camera;
            if (cam == null)
                cam = rec.camera = Misc.GetOrAddComponent<Camera>(rec.go);
            if (m_syncVisibility && dtrans.dataFlags.hasVisibility)
                cam.enabled = dtrans.visibility.visibleInRender;

            cam.orthographic = data.orthographic;

#if UNITY_2018_1_OR_NEWER
            // use physical camera params if available
            if (m_usePhysicalCameraParams && dflags.hasFocalLength && dflags.hasSensorSize)
            {
                cam.usePhysicalProperties = true;
                cam.focalLength = data.focalLength;
                cam.sensorSize = data.sensorSize;
                cam.lensShift = data.lensShift;
#if UNITY_2018_3_OR_NEWER
                //todo: gate fit support
#endif
            }
            else
#endif
            {
                if (dflags.hasFov)
                    cam.fieldOfView = data.fov;
            }

            if (dflags.hasNearPlane && dflags.hasFarPlane)
            {
                cam.nearClipPlane = data.nearPlane;
                cam.farClipPlane = data.farPlane;
            }

            if (dflags.hasViewMatrix)
                cam.worldToCameraMatrix = data.viewMatrix;
            //else
            //    cam.ResetWorldToCameraMatrix();

            if (dflags.hasProjMatrix)
                cam.projectionMatrix = data.projMatrix;
            //else
            //    cam.ResetProjectionMatrix();
            return rec;
        }

        EntityRecord UpdateLight(LightData data)
        {
            if (!m_syncLights)
                return null;

            var dtrans = data.transform;
            var dflags = data.dataFlags;
            var rec = UpdateTransform(dtrans);
            if (rec == null || dflags.unchanged)
                return null;

            var lt = rec.light;
            if (lt == null)
                lt = rec.light = Misc.GetOrAddComponent<Light>(rec.go);

            if (m_syncVisibility && dtrans.dataFlags.hasVisibility)
                lt.enabled = dtrans.visibility.visibleInRender;

            var lightType = data.lightType;
            if ((int)lightType != -1)
                lt.type = data.lightType;
            if (dflags.hasShadowType)
                lt.shadows = data.shadowType;

            if(dflags.hasColor)
                lt.color = data.color;
            if (dflags.hasIntensity)
                lt.intensity = data.intensity;
            if (dflags.hasRange)
                lt.range = data.range;
            if (dflags.hasSpotAngle)
                lt.spotAngle = data.spotAngle;
            return rec;
        }

        void UpdateReference(EntityRecord dst, EntityRecord src)
        {
            if (src.dataType == EntityType.Unknown)
            {
                Debug.LogError("MeshSync: should not be here!");
                return;
            }

            var dstgo = dst.go;
            var srcgo = src.go;
            if (src.dataType == EntityType.Camera)
            {
                var srccam = src.camera;
                if (srccam != null)
                {
                    var dstcam = dst.camera;
                    if (dstcam == null)
                        dstcam = dst.camera = Misc.GetOrAddComponent<Camera>(dstgo);
                    if (m_syncVisibility && dst.hasVisibility)
                        dstcam.enabled = dst.visibility.visibleInRender;
                    dstcam.enabled = srccam.enabled;
                    dstcam.orthographic = srccam.orthographic;
                    dstcam.fieldOfView = srccam.fieldOfView;
                    dstcam.nearClipPlane = srccam.nearClipPlane;
                    dstcam.farClipPlane = srccam.farClipPlane;
                }
            }
            else if (src.dataType == EntityType.Light)
            {
                var srclt = src.light;
                if (srclt != null)
                {
                    var dstlt = dst.light;
                    if (dstlt == null)
                        dstlt = dst.light = Misc.GetOrAddComponent<Light>(dstgo);
                    if (m_syncVisibility && dst.hasVisibility)
                        dstlt.enabled = dst.visibility.visibleInRender;
                    dstlt.type = srclt.type;
                    dstlt.color = srclt.color;
                    dstlt.intensity = srclt.intensity;
                    dstlt.range = srclt.range;
                    dstlt.spotAngle = srclt.spotAngle;
                }
            }
            else if (src.dataType == EntityType.Mesh)
            {
                var mesh = src.mesh;
                var srcmr = src.meshRenderer;
                var srcsmr = src.skinnedMeshRenderer;

                if (mesh != null)
                {
                    var dstpcr = dst.pointCacheRenderer;
                    if (dstpcr != null)
                    {
                        if (m_syncVisibility && dst.hasVisibility)
                            dstpcr.enabled = dst.visibility.visibleInRender;
                        dstpcr.sharedMesh = mesh;

                        if (dstpcr.sharedMaterials == null || dstpcr.sharedMaterials.Length == 0)
                        {
                            Material[] materials = null;
                            if (srcmr != null)
                                materials = srcmr.sharedMaterials;
                            else if (srcsmr != null)
                                materials = srcsmr.sharedMaterials;

                            if (materials != null)
                            {
                                foreach (var m in materials)
                                    m.enableInstancing = true;
                                dstpcr.sharedMaterials = materials;
                            }
                        }
                    }
                    else if (srcmr != null)
                    {
                        var dstmr = dst.meshRenderer;
                        var dstmf = dst.meshFilter;
                        if (dstmr == null)
                        {
                            dstmr = dst.meshRenderer = Misc.GetOrAddComponent<MeshRenderer>(dstgo);
                            dstmf = dst.meshFilter = Misc.GetOrAddComponent<MeshFilter>(dstgo);
                        }

                        if (m_syncVisibility && dst.hasVisibility)
                            dstmr.enabled = dst.visibility.visibleInRender;
                        dstmf.sharedMesh = mesh;
                        dstmr.sharedMaterials = srcmr.sharedMaterials;
                    }
                    else if (srcsmr != null)
                    {
                        var dstsmr = dst.skinnedMeshRenderer;
                        if (dstsmr == null)
                            dstsmr = dst.skinnedMeshRenderer = Misc.GetOrAddComponent<SkinnedMeshRenderer>(dstgo);

                        // disable SkinnedMeshRenderer while updating
                        bool oldEnabled = dstsmr.enabled;
                        dstsmr.enabled = false;
                        dstsmr.sharedMesh = mesh;
                        dstsmr.sharedMaterials = srcsmr.sharedMaterials;
                        dstsmr.bones = srcsmr.bones;
                        dstsmr.rootBone = srcsmr.rootBone;
                        dstsmr.updateWhenOffscreen = srcsmr.updateWhenOffscreen;
                        int blendShapeCount = mesh.blendShapeCount;
                        for (int bi = 0; bi < blendShapeCount; ++bi)
                            dstsmr.SetBlendShapeWeight(bi, srcsmr.GetBlendShapeWeight(bi));

                        if (m_syncVisibility && dst.hasVisibility)
                            dstsmr.enabled = dst.visibility.visibleInRender;
                        else
                            dstsmr.enabled = oldEnabled;
                    }

                    // handle mesh collider
                    if (m_updateMeshColliders)
                    {
                        var srcmc = srcgo.GetComponent<MeshCollider>();
                        if (srcmc != null && srcmc.sharedMesh == mesh)
                        {
                            var dstmc = Misc.GetOrAddComponent<MeshCollider>(dstgo);
                            dstmc.enabled = srcmc.enabled;
                            dstmc.isTrigger = srcmc.isTrigger;
                            dstmc.sharedMaterial = srcmc.sharedMaterial;
                            dstmc.sharedMesh = mesh;
                            dstmc.convex = srcmc.convex;
                            dstmc.cookingOptions = srcmc.cookingOptions;
                        }
                    }
                }
            }
            else if (src.dataType == EntityType.Points)
            {
                var srcpcr = src.pointCacheRenderer;
                if (srcpcr != null)
                {
                    var dstpcr = dst.pointCacheRenderer;
                    dstpcr.sharedMesh = srcpcr.sharedMesh;

                    var materials = srcpcr.sharedMaterials;
                    for (int i = 0; i < materials.Length; ++i)
                        materials[i].enableInstancing = true;
                    dstpcr.sharedMaterials = materials;
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
            if (!m_handleAssets)
                return;

            clipData.Convert(m_animationInterpolation);
            if (m_keyframeReduction)
                clipData.KeyframeReduction(m_reductionThreshold, m_reductionEraseFlatCurves);

            //float start = Time.realtimeSinceStartup;

            var animClipCache = new Dictionary<GameObject, AnimationClip>();

            int numAnimations = clipData.numAnimations;
            for (int ai = 0; ai < numAnimations; ++ai)
            {
                AnimationData data = clipData.GetAnimation(ai);

                var path = data.path;
                EntityRecord rec = null;
                m_clientObjects.TryGetValue(path, out rec);

                Transform target = null;
                if (rec != null)
                    target = rec.trans;
                if (target == null)
                {
                    bool dummy = false;
                    target = FindOrCreateObjectByPath(path, true, ref dummy);
                    if (target == null)
                        return;
                }

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
                        SaveAsset(ref clip, dstPath);
                        animator.runtimeAnimatorController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPathWithClip(dstPath + ".controller", clip);
                        animClipCache[root.gameObject] = clip;
                    }
                    clip.frameRate = clipData.frameRate;
                }

                var animPath = path.Replace("/" + root.name, "");
                if (animPath.Length > 0 && animPath[0] == '/')
                    animPath = animPath.Remove(0, 1);

                // get animation curves
                var ctx = new AnimationImportContext()
                {
                    clip = clip,
                    root = root.gameObject,
                    target = target.gameObject,
                    path = animPath,
                    enableVisibility = m_syncVisibility,
#if UNITY_2018_1_OR_NEWER
                    usePhysicalCameraParams = m_usePhysicalCameraParams,
#endif
                };
                if (rec != null)
                {
                    if (rec.meshRenderer != null) ctx.mainComponentType = typeof(MeshRenderer);
                    else if (rec.skinnedMeshRenderer != null) ctx.mainComponentType = typeof(SkinnedMeshRenderer);
                }
                data.ExportToClip(ctx);
            }

            //Debug.Log("UpdateAnimation() " + (Time.realtimeSinceStartup - start) + " sec");

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

        public void AssignMaterials(EntityRecord rec)
        {
            if (rec.go == null || rec.mesh == null || rec.materialIDs == null)
                return;

            var r = rec.meshRenderer != null ? (Renderer)rec.meshRenderer : (Renderer)rec.skinnedMeshRenderer;
            if (r == null)
                return;

            bool changed = false;
            int materialCount = rec.materialIDs.Length;
            var materials = new Material[materialCount];
            var prevMaterials = r.sharedMaterials;
            Array.Copy(prevMaterials, materials, Math.Min(prevMaterials.Length, materials.Length));

            for (int si = 0; si < materialCount; ++si)
            {
                var mid = rec.materialIDs[si];
                var material = FindMaterial(mid);
                if (material != null)
                {
                    if (materials[si] != material)
                    {
                        materials[si] = material;
                        changed = true;
                    }
                }
                else if (materials[si] == null)
                {
                    // assign dummy material to prevent to go pink
                    if (m_dummyMaterial == null)
                    {
                        m_dummyMaterial = CreateDefaultMaterial();
                        m_dummyMaterial.name = "Dummy";
                    }
                    materials[si] = m_dummyMaterial;
                    changed = true;
                }
            }

            if (changed)
            {
#if UNITY_EDITOR
                if (m_recordAssignMaterials)
                    Undo.RecordObject(r, "Assign Material");
#endif
                r.sharedMaterials = materials;
            }
        }

        public bool EraseEntityRecord(Identifier identifier)
        {
            var id = identifier.id;
            var path = identifier.name;

            GameObject target = null;
            EntityRecord rec = null;
            bool ret = false;
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
                ret = m_clientObjects.Remove(path);
            }

            if (target != null && !IsAsset(target))
            {
                if (onDeleteEntity != null)
                    onDeleteEntity.Invoke(target);
                DestroyImmediate(target);
            }
            return ret;
        }
        #endregion

        #region Tools
        public bool ApplyMaterialList(MaterialList ml)
        {
            if (ml == null || ml.materials == null || ml.materials.Count == 0)
                return false;

            bool updated = false;
            int materialCount = Mathf.Min(m_materialList.Count, ml.materials.Count);
            for (int mi = 0; mi < materialCount; ++mi)
            {
                var src = ml.materials[mi];
                if (src.material != null)
                {
                    var dst = m_materialList.Find(a => a.id == src.id);
                    if (dst != null)
                    {
                        dst.material = src.material;
                        updated = true;
                    }
                }
            }
            if (updated)
                ReassignMaterials();
            return updated;
        }

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
        }
        public void GenerateLightmapUV()
        {
            foreach (var kvp in m_clientObjects)
                GenerateLightmapUV(kvp.Value.go);
            foreach (var kvp in m_hostObjects)
            {
                if (kvp.Value.mesh != null)
                    GenerateLightmapUV(kvp.Value.go);
            }
        }


        public void ExportMaterials(bool overwrite = true, bool useExistingOnes = false)
        {
            MakeSureAssetDirectoryExists();

            // need to avoid filename collision
            var nameGenerator = new Misc.UniqueNameGenerator();
            var basePath = assetPath;

            Func<Material, Material> doExport = (Material mat) => {
                if (mat == null || IsAsset(mat))
                    return mat;

                var dstPath = string.Format("{0}/{1}.mat", basePath, nameGenerator.Gen(mat.name));
                var existing = AssetDatabase.LoadAssetAtPath<Material>(dstPath);
                if (overwrite || existing == null)
                {
                    SaveAsset(ref mat, dstPath);
                    if (m_logging)
                        Debug.Log("exported material " + dstPath);
                }
                else if (useExistingOnes && existing != null)
                    mat = existing;
                return mat;
            };

            foreach (var m in m_materialList)
                m.material = doExport(m.material); // material maybe updated by SaveAsset()
            m_dummyMaterial = doExport(m_dummyMaterial);

            AssetDatabase.SaveAssets();
            ReassignMaterials();
        }

        public void ExportMeshes(bool overwrite = true, bool useExistingOnes = false)
        {
            MakeSureAssetDirectoryExists();

            // need to avoid filename collision
            var nameGenerator = new Misc.UniqueNameGenerator();
            var basePath = assetPath;

            // export client meshes
            foreach (var kvp in m_clientObjects)
            {
                var mesh = kvp.Value.mesh;
                if (mesh == null || IsAsset(mesh))
                    continue;

                var dstPath = string.Format("{0}/{1}.asset", basePath, nameGenerator.Gen(mesh.name));
                var existing = AssetDatabase.LoadAssetAtPath<Mesh>(dstPath);
                if (overwrite || existing == null)
                {
                    SaveAsset(ref mesh, dstPath);
                    kvp.Value.mesh = mesh; // mesh maybe updated by SaveAsset()
                    if (m_logging)
                        Debug.Log("exported material " + dstPath);
                }
                else if (useExistingOnes && existing != null)
                    kvp.Value.mesh = existing;
            }

            // replace existing meshes
            int n = 0;
            foreach (var kvp in m_hostObjects)
            {
                var rec = kvp.Value;
                if (rec.go == null || rec.mesh == null)
                    continue;

                rec.origMesh.Clear(); // make editor can recognize mesh has modified by CopySerialized()
                EditorUtility.CopySerialized(rec.mesh, rec.origMesh);
                rec.mesh = null;

                var mf = rec.go.GetComponent<MeshFilter>();
                if (mf != null)
                    mf.sharedMesh = rec.origMesh;

                var smr = rec.go.GetComponent<SkinnedMeshRenderer>();
                if (smr != null)
                    smr.sharedMesh = rec.origMesh;

                if (m_logging)
                    Debug.Log("updated mesh " + rec.origMesh.name);
                ++n;
            }
            if (n > 0)
                AssetDatabase.SaveAssets();
        }

        public bool ExportMaterialList(string path)
        {
            if (path == null || path.Length == 0)
                return false;

            path = path.Replace(Application.dataPath, "Assets/");
            if (!path.EndsWith(".asset"))
                path = path + ".asset";

            var ml = ScriptableObject.CreateInstance<MaterialList>();
            ml.materials = m_materialList;
            AssetDatabase.CreateAsset(ml, path);
            return true;
        }

        public bool ImportMaterialList(string path)
        {
            if (path == null || path.Length == 0)
                return false;

            path = path.Replace(Application.dataPath, "Assets/");

            var ml = AssetDatabase.LoadAssetAtPath<MaterialList>(path);
            return ApplyMaterialList(ml);
        }

        public void CheckMaterialAssigned(bool recordUndo = true)
        {
            bool changed = false;
            foreach (var kvp in m_clientObjects)
            {
                var rec = kvp.Value;
                if (rec.go != null && rec.go.activeInHierarchy)
                {
                    var mr = rec.go.GetComponent<Renderer>();
                    if (mr == null || rec.mesh == null)
                        continue;

                    var materials = mr.sharedMaterials;
                    int n = Math.Min(materials.Length, rec.materialIDs.Length);
                    for (int si = 0; si < n; ++si)
                    {
                        int mid = rec.materialIDs[si];
                        var mrec = m_materialList.Find(a => a.id == mid);
                        if (mrec != null && materials[si] != mrec.material)
                        {
                            mrec.material = materials[si];
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
                int group = 0;
                if (recordUndo)
                {
                    // assume last undo group is "Assign Material" performed by mouse drag & drop.
                    // collapse reassigning materials into it.
                    group = Undo.GetCurrentGroup() - 1;
                    m_recordAssignMaterials = true;
                }
                ReassignMaterials();
                if (recordUndo)
                {
                    m_recordAssignMaterials = false;
                    Undo.CollapseUndoOperations(group);
                    Undo.FlushUndoRecordObjects();
                }
                ForceRepaint();
            }
        }

        public void AssignMaterial(MaterialHolder holder, Material mat, bool recordUndo = true)
        {
            if (recordUndo)
            {
                Undo.RegisterCompleteObjectUndo(this, "Assign Material");
                m_recordAssignMaterials = true;
            }
            holder.material = mat;
            ReassignMaterials();
            if (recordUndo)
            {
                m_recordAssignMaterials = false;
                Undo.FlushUndoRecordObjects();
            }
            ForceRepaint();
        }

        public List<AnimationClip> GetAnimationClips()
        {
            var ret = new List<AnimationClip>();

            Action<GameObject> gatherClips = (go) => {
                AnimationClip[] clips = null;
#if UNITY_2018_3_OR_NEWER
                clips = AnimationUtility.GetAnimationClips(go);
#else
                var animator = go.GetComponent<Animator>();
                if (animator != null && animator.runtimeAnimatorController != null)
                    clips = animator.runtimeAnimatorController.animationClips;
#endif
                if (clips != null && clips.Length > 0)
                    ret.AddRange(clips);
            };

            gatherClips(gameObject);
            // process children. root children should be enough.
            var trans = transform;
            int childCount = trans.childCount;
            for (int ci = 0; ci < childCount; ++ci)
                gatherClips(transform.GetChild(ci).gameObject);
            return ret;
        }

        protected void OnSceneViewGUI(SceneView sceneView)
        {
            if (m_syncMaterialList)
            {
                if (Event.current.type == EventType.DragExited && Event.current.button == 0)
                    CheckMaterialAssigned();
            }
        }
#endif
        #endregion

        #region Events
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
#endif

        void OnDestroy()
        {
            m_tmpI.Dispose();
            m_tmpV2.Dispose();
            m_tmpV3.Dispose();
            m_tmpV4.Dispose();
            m_tmpC.Dispose();
        }

        protected virtual void OnEnable()
        {
#if UNITY_EDITOR
#if UNITY_2019_1_OR_NEWER
            SceneView.duringSceneGui += OnSceneViewGUI;
#else
            SceneView.onSceneGUIDelegate += OnSceneViewGUI;
#endif
#endif
        }

        protected virtual void OnDisable()
        {
#if UNITY_EDITOR
#if UNITY_2019_1_OR_NEWER
            SceneView.duringSceneGui -= OnSceneViewGUI;
#else
            SceneView.onSceneGUIDelegate -= OnSceneViewGUI;
#endif
#endif
        }
        #endregion
    }
}
