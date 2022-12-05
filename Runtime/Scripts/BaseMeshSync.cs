using System;
using System.Linq;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.Animations;
using Unity.Collections;
using UnityEngine.Assertions;
using System.IO;
using System.Runtime.CompilerServices;
using JetBrains.Annotations;
using Unity.FilmInternalUtilities;

#if AT_USE_SPLINES
using Unity.Mathematics;
#endif

#if AT_USE_HDRP
using UnityEngine.Rendering; //Volume
using UnityEngine.Rendering.HighDefinition;
using UnityEngine.Experimental.Rendering;
#endif

#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.SceneManagement;
using Unity.FilmInternalUtilities.Editor;
#endif


[assembly: InternalsVisibleTo("Unity.Utilities.VariantExport")]

namespace Unity.MeshSync
{
/// <summary>
/// A delegate to handle scene updates
/// </summary>
internal delegate void SceneHandler();

/// <summary>
/// A delegate to handle audio updates
/// </summary>
internal delegate void UpdateAudioHandler(AudioClip audio, AudioData data);

/// <summary>
/// A delegate to handle texture updates
/// </summary>
internal delegate void UpdateTextureHandler(Texture2D tex, TextureData data);

/// <summary>
/// A delegate to handle material updates
/// </summary>
internal delegate void UpdateMaterialHandler(Material mat, MaterialData data);

/// <summary>
/// A delegate to handle entity updates
/// </summary>
internal delegate void UpdateEntityHandler(GameObject obj, TransformData data);

/// <summary>
/// A delegate to handle animation updates
/// </summary>
internal delegate void UpdateAnimationHandler(AnimationClip anim, AnimationClipData data);

/// <summary>
/// A delegate to handle entity deletions
/// </summary>
internal delegate void DeleteEntityHandler(GameObject obj);

/// <summary>
/// A delegate to handle instance info updates
/// </summary>
/// <param name="data"></param>
internal delegate void UpdateInstanceInfoHandler(string path, GameObject go, Matrix4x4[] transforms);

/// <summary>
/// A delegate to handle instance meshes.
/// </summary>
internal delegate void UpdateInstancedEntityHandler(string path, GameObject go);

/// <summary>
/// A delegate to handle instance deletions
/// </summary>
internal delegate void DeleteInstanceHandler(string path);

/// <summary>
/// Internal analytics observer data
/// </summary>
public struct MeshSyncAnalyticsData {
    internal MeshSyncSessionStartAnalyticsData? sessionStartData;
    internal MeshSyncSyncAnalyticsData?         syncData;
}

/// <summary>
/// Data about sync.
/// </summary>
public struct MeshSyncSyncAnalyticsData {
    internal AssetType  assetType;
    internal EntityType entityType;
    internal string     syncMode;
}

/// <summary>
/// Information about the DCC tool used with MeshSync.
/// </summary>
public struct MeshSyncSessionStartAnalyticsData {
    public string DCCToolName;
}

    //----------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// The base class of main MeshSync components (MeshSyncServer, SceneCachePlayer),
    /// which encapsulates common functionalities
    /// </summary>
    [ExecuteInEditMode]
    public abstract partial class BaseMeshSync : MonoBehaviour, IObservable<MeshSyncAnalyticsData>, ISerializationCallbackReceiver {


        #region EventHandler Declarations

        /// <summary>
        /// An event that is executed when the scene update is started
        /// </summary>
        internal event SceneHandler onSceneUpdateBegin;

        /// <summary>
        /// An event that is executed when an audio is updated
        /// </summary>
        internal event UpdateAudioHandler onUpdateAudio;

        /// <summary>
        /// An event that is executed when a texture is updated
        /// </summary>
        internal event UpdateTextureHandler onUpdateTexture;

        /// <summary>
        /// An event that is executed when an material is updated
        /// </summary>
        internal event UpdateMaterialHandler onUpdateMaterial;

        /// <summary>
        /// An event that is executed when an entity is updated
        /// </summary>
        internal event UpdateEntityHandler onUpdateEntity;

        /// <summary>
        /// An event that is executed when an animation is updated
        /// </summary>
        internal event UpdateAnimationHandler onUpdateAnimation;

        /// <summary>
        /// An event that is executed when an entity is deleted
        /// </summary>
        internal event DeleteEntityHandler onDeleteEntity;

        /// <summary>
        /// An event that is executed when the scene update is finished
        /// </summary>
        internal event SceneHandler onSceneUpdateEnd;

        internal event UpdateInstanceInfoHandler onUpdateInstanceInfo;

        internal event UpdateInstancedEntityHandler onUpdateInstancedEntity;

        internal event DeleteInstanceHandler onDeleteInstanceInfo;

        internal event DeleteInstanceHandler onDeleteInstancedEntity;

        #endregion EventHandler Declarations

        //----------------------------------------------------------------------------------------------------------------------

        internal void Init(string assetsFolder) {
            Assert.IsTrue(assetsFolder.StartsWith("Assets"));
            m_assetsFolder = assetsFolder.Replace('\\', '/');
            m_rootObject = gameObject.transform;

            m_materialList.Clear();
            m_textureList.Clear();
            m_audioList.Clear();

            m_clientObjects.Clear();
            m_hostObjects.Clear();
            m_objIDTable.Clear();

            m_clientInstances.Clear();
            m_clientInstancedEntities.Clear();
            
            m_prefabDict.Clear();

            InitInternalV();
        }

        private protected abstract void InitInternalV();

        //----------------------------------------------------------------------------------------------------------------------

        #region Getter/Setter

        internal string GetAssetsFolder() { return m_assetsFolder; }

        public void SetAssetsFolder(string folder) { m_assetsFolder = folder; }

        internal Transform GetRootObject() { return m_rootObject; }

        internal void SetRootObject(Transform t) { m_rootObject = t; }


        internal IDictionary<string, EntityRecord> GetClientObjects() {
            return m_clientObjects;
        }

        #endregion Simple Getter/Setter

        #region Properties
        
        [SerializeField] private int  currentSessionId = -1;
        
        private bool forceDeleteChildrenInNextSession = false;

        private protected string GetServerDocRootPath() { return Application.streamingAssetsPath + "/MeshSyncServerRoot"; }

        private protected void SetSaveAssetsInScene(bool saveAssetsInScene) { m_saveAssetsInScene = saveAssetsInScene; }

        private protected void MarkMeshesDynamic(bool markMeshesDynamic) { m_markMeshesDynamic = markMeshesDynamic; }

        internal void EnableKeyValuesSerialization(bool kvEnabled) { m_keyValuesSerializationEnabled = kvEnabled; }

        internal abstract MeshSyncPlayerConfig GetConfigV();


        internal bool useCustomCameraMatrices
        {
            get { return m_useCustomCameraMatrices; }
            set { m_useCustomCameraMatrices = value; }
        }

        internal List<MaterialHolder> materialList { get { return m_materialList; } }
        internal List<TextureHolder>  textureList  { get { return m_textureList; } }
        internal PrefabDictionary     prefabDict   { get { return m_prefabDict; } }


#if UNITY_EDITOR

        protected void SetSortEntities(bool sortEntities) { m_sortEntities = sortEntities; }

        internal bool foldSyncSettings
        {
            get { return m_foldSyncSettings; }
            set { m_foldSyncSettings = value; }
        }
        internal bool foldImportSettings
        {
            get { return m_foldImportSettings; }
            set { m_foldImportSettings = value; }
        }
        internal bool foldMisc
        {
            get { return m_foldMisc; }
            set { m_foldMisc = value; }
        }
        internal bool foldMaterialList
        {
            get { return m_foldMaterialList; }
            set { m_foldMaterialList = value; }
        }
        internal bool foldAnimationTweak
        {
            get { return m_foldAnimationTweak; }
            set { m_foldAnimationTweak = value; }
        }
        internal bool foldExportAssets
        {
            get { return m_foldExportAssets; }
            set { m_foldExportAssets = value; }
        }
#endif
        #endregion

        //----------------------------------------------------------------------------------------------------------------------

        #region Impl
        void SerializeDictionary<K, V>(Dictionary<K, V> dic, ref K[] keys, ref V[] values)
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

        //----------------------------------------------------------------------------------------------------------------------

        /// <summary>
        /// Called during serialization as an implementation of ISerializationCallbackReceiver
        /// </summary>
        public void OnBeforeSerialize() {
            OnBeforeSerializeMeshSyncPlayerV();

#if UNITY_EDITOR
            SaveMaterialRenderTexturesToAssetDatabase();
#endif

            if (!m_keyValuesSerializationEnabled)
                return;

            SerializeDictionary(m_clientObjects, ref m_clientObjects_keys, ref m_clientObjects_values);
            SerializeDictionary(m_hostObjects, ref m_hostObjects_keys, ref m_hostObjects_values);
            SerializeDictionary(m_objIDTable, ref m_objIDTable_keys, ref m_objIDTable_values);

            m_baseMeshSyncVersion = CUR_BASE_MESHSYNC_VERSION;
        }


        /// <summary>
        /// Called during serialization as an implementation of ISerializationCallbackReceiver
        /// </summary>
        public void OnAfterDeserialize() {
            DeserializeDictionary(m_clientObjects, ref m_clientObjects_keys, ref m_clientObjects_values);
            DeserializeDictionary(m_hostObjects, ref m_hostObjects_keys, ref m_hostObjects_values);
            DeserializeDictionary(m_objIDTable, ref m_objIDTable_keys, ref m_objIDTable_values);

            OnAfterDeserializeMeshSyncPlayerV();

            if (CUR_BASE_MESHSYNC_VERSION == m_baseMeshSyncVersion)
                return;

            if (m_baseMeshSyncVersion < (int)BaseMeshSyncVersion.INITIAL_0_10_0) {
#pragma warning disable 612
                foreach (MaterialHolder m in m_materialList) {
                    if (m.materialIID != 0)
                        continue;

                    m.ShouldApplyMaterialData = false;
                }

                MeshSyncPlayerConfig config = GetConfigV();
                config?.UsePhysicalCameraParams(m_usePhysicalCameraParams);
#pragma warning restore 612

            }

            m_baseMeshSyncVersion = CUR_BASE_MESHSYNC_VERSION;

        }

        private protected abstract void OnBeforeSerializeMeshSyncPlayerV();
        private protected abstract void OnAfterDeserializeMeshSyncPlayerV();

        //----------------------------------------------------------------------------------------------------------------------
        #endregion

        #region Misc
        //[MethodImpl(MethodImplOptions.AggressiveInlining)]
        private protected bool Try(Action act) {
            try {
                act.Invoke();
                return true;
            } catch (Exception e) {
                if (GetConfigV().Logging)
                    Debug.LogError(e);
                return false;
            }
        }

        //----------------------------------------------------------------------------------------------------------------------    
        private void MakeSureAssetDirectoryExists() {
#if UNITY_EDITOR
            if (Directory.Exists(m_assetsFolder))
                return;
            
            Directory.CreateDirectory(m_assetsFolder);
            AssetDatabase.Refresh();
#endif
        }

        //----------------------------------------------------------------------------------------------------------------------    

        private bool IsAsset(UnityEngine.Object obj)
        {
#if UNITY_EDITOR
            return AssetDatabase.Contains(obj);
#else
        return false;
#endif
        }

        private bool DestroyIfNotAsset(UnityEngine.Object obj)
        {
            if (obj != null && IsAsset(obj))
            {
                DestroyImmediate(obj, false);
                return true;
            }
            return false;
        }

        internal string BuildPath(Transform t)
        {
            Transform parent = t.parent;
            if (parent != null && parent != m_rootObject)
                return BuildPath(parent) + "/" + t.name;
            else
                return "/" + t.name;
        }

        internal static Texture2D FindTexture(int id, List<TextureHolder> textureHolders)
        {
            if (id == Lib.invalidID)
                return null;
            TextureHolder rec = textureHolders.Find(a => a.id == id);
            return rec != null ? rec.texture : null;
        }

        internal Material FindMaterial(int id)
        {
            if (id == Lib.invalidID)
                return null;
            MaterialHolder rec = m_materialList.Find(a => a.id == id);
            return rec != null ? rec.material : null;
        }

        internal bool EraseMaterialRecord(int id)
        {
            return m_materialList.RemoveAll(v => v.id == id) != 0;
        }

        private protected int GetMaterialIndex(Material mat)
        {
            if (mat == null)
                return Lib.invalidID;

            for (int i = 0; i < m_materialList.Count; ++i)
            {
                if (m_materialList[i].material == mat)
                    return i;
            }

            int ret = m_materialList.Count;
            MaterialHolder tmp = new MaterialHolder();
            tmp.name = mat.name;
            tmp.material = mat;
            tmp.id = ret + 1;
            m_materialList.Add(tmp);
            return ret;
        }

        internal AudioClip FindAudio(int id)
        {
            if (id == Lib.invalidID)
                return null;
            AudioHolder rec = m_audioList.Find(a => a.id == id);
            return rec != null ? rec.audio : null;
        }

        internal int GetObjectlID(GameObject go) {
            if (go == null)
                return Lib.invalidID;

            if (m_objIDTable.TryGetValue(go, out int ret))
                return ret;

            ret = ++m_objIDSeed;
            m_objIDTable[go] = ret;
            return ret;
        }

        //----------------------------------------------------------------------------------------------------------------------    


        private static Material CreateDefaultMaterial(string shaderName = null) {
            Material mat = new Material(GetShader(shaderName, out _));
            UpdateShader(mat, shaderName);
            return mat;
        }

        internal void ForceRepaint() {
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

        internal void BeforeUpdateScene(FenceMessage? mes = null) {
            onSceneUpdateBegin?.Invoke();

            CheckForNewSession(mes);

            numberOfPropertiesReceived = 0;
        }

        public void ForceDeleteChildrenInNextSession() {
            forceDeleteChildrenInNextSession = true;
        }

        void CheckForNewSession(FenceMessage? mes) {
#if UNITY_EDITOR
            if (!mes.HasValue || currentSessionId == mes.Value.SessionId) {
                return;
            }

            currentSessionId = mes.Value.SessionId;

            SendEventData(new MeshSyncAnalyticsData()
                { sessionStartData = new MeshSyncSessionStartAnalyticsData() { DCCToolName = mes.Value.DCCToolName } });

            if (transform.childCount <= 0) {
                return;
            }

            int choice;

            if (forceDeleteChildrenInNextSession) {
                choice = 2;
                forceDeleteChildrenInNextSession = false;
            }
            else {
                choice = EditorUtility.DisplayDialogComplex("A new session started.",
                    "MeshSync detected that the DCC tool session has changed. To ensure that the sync state is correct you can delete previously synced objects, stash them and move them to another game object or ignore this and keep all children.",
                    "Ignore and keep all children",
                    "Stash",
                    "Delete all children of the server");
            }

            switch (choice) {
                case 1:
                    // Stash
                    var stash = new GameObject($"{name}_stash").transform;
                    for (int i = transform.childCount - 1; i >= 0; i--) {
                        Transform child = transform.GetChild(i);
                        child.SetParent(stash, true);
                    }

                    Init(GetAssetsFolder());
                    break;
                case 2:
                    // Destroy
                    transform.DestroyChildrenImmediate();

                    Init(GetAssetsFolder());
                    break;
            }
#endif
        }

        private protected void UpdateScene(SceneData scene, bool updateNonMaterialAssets, bool logAnalytics = true) {
            MeshSyncPlayerConfig config = GetConfigV();
            // handle assets
            Try(() => {
                int numAssets = scene.numAssets;
                if (numAssets > 0) {
                    bool save = false;
                    for (int i = 0; i < numAssets; ++i) {
                        AssetData asset = scene.GetAsset(i);

                        //Only update non-MaterialAsset if specified
                        if (!updateNonMaterialAssets && asset.type != AssetType.Material)
                            continue;

                        switch (asset.type) {
                            case AssetType.File:
                                UpdateFileAsset((FileAssetData)asset);
                                break;
                            case AssetType.Audio:
                                UpdateAudioAsset((AudioData)asset);
                                break;
                            case AssetType.Texture:
                                UpdateTextureAsset((TextureData)asset);
                                break;
                            case AssetType.Material:
                                UpdateMaterialAssetV((MaterialData)asset);
                                break;
                            case AssetType.Animation:
                                UpdateAnimationAsset((AnimationClipData)asset, config);
                                save = true;
                                break;
                            default:
                                if (config.Logging)
                                    Debug.Log("unknown asset: " + asset.name);
                                break;
                        }

                        if (logAnalytics) {
                            string syncMode = "None";
                            if (asset.type == AssetType.Material) {
                                syncMode = scene.GetMaterialSyncMode();
                                
                                // TODO: Don't do this when GetMaterialSyncMode() works
                                if (textureList.Count > 0) {
                                    syncMode = "Basic";
                                }
                            }

                            SendEventData(new MeshSyncAnalyticsData() { syncData = new MeshSyncSyncAnalyticsData() { assetType = asset.type, syncMode = syncMode }});
                        }
                    }
#if UNITY_EDITOR
                    if (save)
                        AssetDatabase.SaveAssets();
#endif
                }
            });

            // handle entities
            Try(() => {
                int numObjects = scene.numEntities;
                for (int i = 0; i < numObjects; ++i) {
                    EntityRecord dst = null;
                    TransformData src = scene.GetEntity(i);
                    switch (src.entityType) {
                        case EntityType.Transform:
                            dst = UpdateTransformEntity(src, config);
                            break;
                        case EntityType.Camera:
                            dst = UpdateCameraEntity((CameraData)src, config);
                            break;
                        case EntityType.Light:
                            dst = UpdateLightEntity((LightData)src, config);
                            break;
                        case EntityType.Mesh:
                            dst = UpdateMeshEntity((MeshData)src, config);
                            break;
                        case EntityType.Points:
                            dst = UpdatePointsEntity((PointsData)src, config);
                            break;
                        case EntityType.Curve:
                            dst = UpdateCurveEntity((CurvesData)src, config);
                            break;
                        default:
                            Debug.LogError($"Unhandled entity type: {src.entityType}");
                            break;
                    }

                    SendEventData(new MeshSyncAnalyticsData() { syncData = new MeshSyncSyncAnalyticsData(){ entityType = src.entityType} });


                    if (dst != null && onUpdateEntity != null)
                        onUpdateEntity.Invoke(dst.go, src);
                }
            });

            // handle constraints
            Try(() => {
                int numConstraints = scene.numConstraints;
                for (int i = 0; i < numConstraints; ++i)
                    UpdateConstraint(scene.GetConstraint(i));
            });

            // handle instance meshes
            Try(() =>
            {
                var numMeshes = scene.numInstancedEntities;
                for (var i = 0; i < numMeshes; ++i) {
                    var src = scene.GetInstancedEntity(i);
                    var dst = UpdateInstancedEntity(src);

                    if (dst == null)
                    {
                        return;
                    }

                    if (onUpdateInstancedEntity != null)
                        onUpdateInstancedEntity(src.path, dst.go);
                }
            });

            // handle instances
            Try(() =>
            {
                var numInstances = scene.numInstanceInfos;
                for (var i = 0; i < numInstances; ++i) {
                    var src = scene.GetInstanceInfo(i);
                    var dst = UpdateInstanceInfo(src);
                    if (onUpdateInstanceInfo != null)
                        onUpdateInstanceInfo.Invoke(src.path, dst.go, src.transforms);
                }
            });

            UpdateProperties(scene);

#if UNITY_EDITOR
            if (config.ProgressiveDisplay)
                ForceRepaint();
#endif

#if AT_USE_HDRP && UNITY_2021_2_OR_NEWER
            if (m_needToResetPathTracing) {
                HDRPUtility.ResetPathTracing();
                m_needToResetPathTracing = false;
            }
#endif
        }

        internal virtual void AfterUpdateScene()
        {
            // If none of the set messages had properties, we need to remove all properties:
            if (numberOfPropertiesReceived == 0) {
                propertyInfos.Clear();
            }

            List<string> deadKeys = null;

            // resolve bones
            foreach (KeyValuePair<string, EntityRecord> kvp in m_clientObjects)
            {
                EntityRecord rec = kvp.Value;
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

                    SkinnedMeshRenderer smr = rec.skinnedMeshRenderer;
                    if (rec.bonePaths != null && rec.bonePaths.Length > 0)
                    {
                        int boneCount = rec.bonePaths.Length;

                        Transform[] bones = new Transform[boneCount];
                        for (int bi = 0; bi < boneCount; ++bi)
                            bones[bi] = FilmInternalUtilities.GameObjectUtility.FindByPath(m_rootObject, rec.bonePaths[bi]);

                        Transform root = null;
                        if (!string.IsNullOrEmpty(rec.rootBonePath))
                            root = FilmInternalUtilities.GameObjectUtility.FindByPath(m_rootObject, rec.rootBonePath);

                        if (root == null && boneCount > 0)
                        {
                            // find root bone
                            root = bones[0];
                            for (; ; )
                            {
                                Transform parent = root.parent;
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
                foreach (string key in deadKeys)
                    m_clientObjects.Remove(key);

            // resolve references
            // this must be another pass because resolving bones can affect references
            foreach (KeyValuePair<string, EntityRecord> kvp in m_clientObjects)
            {
                EntityRecord rec = kvp.Value;
                if (!string.IsNullOrEmpty(rec.reference))
                {
                    EntityRecord srcrec = null;
                    if (m_clientObjects.TryGetValue(rec.reference, out srcrec) && srcrec.go != null)
                    {
                        rec.materialIDs = srcrec.materialIDs;
                        UpdateReference(rec, srcrec, GetConfigV());
                    }
                }
            }

            // reassign materials
            if (m_needReassignMaterials)
            {
                m_materialList = m_materialList.OrderBy(v => v.index).ToList();
                ReassignMaterials(recordUndo: false);
                m_needReassignMaterials = false;
            }

#if UNITY_EDITOR
            // sort objects by index
            if (m_sortEntities) {
                IOrderedEnumerable<EntityRecord> rec = m_clientObjects.Values.OrderBy(v => v.index);
                foreach (EntityRecord r in rec) {
                    if (r.go != null)
                        r.go.GetComponent<Transform>().SetSiblingIndex(r.index + 1000);
                }
            }

            if (!EditorApplication.isPlaying || !EditorApplication.isPaused) {
                // force recalculate skinning
                foreach (KeyValuePair<string, EntityRecord> kvp in m_clientObjects)
                {
                    EntityRecord rec = kvp.Value;
                    SkinnedMeshRenderer smr = rec.skinnedMeshRenderer;
                    if (smr != null && rec.smrEnabled && rec.go.activeInHierarchy) {
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
            MeshSyncLogger.VerboseLog( $"Scene updated.");

            if (onSceneUpdateEnd != null)
                onSceneUpdateEnd.Invoke();
        }

        //----------------------------------------------------------------------------------------------------------------------

        void UpdateFileAsset(FileAssetData src) {
            MakeSureAssetDirectoryExists();
#if UNITY_EDITOR
            src.WriteToFile(m_assetsFolder + "/" + src.name);
#endif
        }

        void UpdateAudioAsset(AudioData src) {
            MakeSureAssetDirectoryExists();
            AudioClip ac = null;

            AudioFormat format = src.format;
            if (format == AudioFormat.RawFile)
            {
#if UNITY_EDITOR
                // create file and import it
                string dstPath = m_assetsFolder + "/" + src.name;
                src.WriteToFile(dstPath);
                AssetDatabase.ImportAsset(dstPath);
                ac = AssetDatabase.LoadAssetAtPath<AudioClip>(dstPath);
                if (ac != null) {
                    AudioImporter importer = (AudioImporter)AssetImporter.GetAtPath(dstPath);
                    if (importer != null) {
                        // nothing todo for now
                    }
                }
#endif
            }
            else
            {
#if UNITY_EDITOR
                // export as .wav and import it
                string dstPath = m_assetsFolder + "/" + src.name + ".wav";
                if (src.ExportAsWave(dstPath)) {
                    AssetDatabase.ImportAsset(dstPath);
                    ac = AssetDatabase.LoadAssetAtPath<AudioClip>(dstPath);
                    if (ac != null) {
                        AudioImporter importer = (AudioImporter)AssetImporter.GetAtPath(dstPath);
                        if (importer != null) {
                            // nothing todo for now
                        }
                    }
                }
#endif
                if (ac == null) {
                    ac = AudioClip.Create(src.name, src.sampleLength, src.channels, src.frequency, false);
                    ac.SetData(src.samples, 0);
                }
            }

            if (ac != null) {
                int id = src.id;
                AudioHolder dst = m_audioList.Find(a => a.id == id);
                if (dst == null) {
                    dst = new AudioHolder();
                    dst.id = id;
                    m_audioList.Add(dst);
                }
                dst.audio = ac;
                if (onUpdateAudio != null)
                    onUpdateAudio.Invoke(ac, src);
            }
        }

        //----------------------------------------------------------------------------------------------------------------------

        void UpdateTextureAsset(TextureData src) {
            MakeSureAssetDirectoryExists();
            Texture2D texture = null;
#if UNITY_EDITOR
            Action<string> doImport = (path) => {
                bool assetExisted = AssetDatabase.LoadAssetAtPath<Texture2D>(path) != null;

                AssetDatabase.ImportAsset(path);
                texture = AssetDatabase.LoadAssetAtPath<Texture2D>(path);
                if (texture != null) {
                    TextureImporter importer = (TextureImporter)AssetImporter.GetAtPath(path);

                    // Make sure the full texture is used unless the user changed this setting before:
                    if (!assetExisted) {
                        importer.npotScale = TextureImporterNPOTScale.None;

                        importer.SaveAndReimport();
                        AssetDatabase.Refresh();
                    }

                    if (importer != null) {
                        switch (src.type) {
                            case TextureType.NormalMap:
                                importer.textureType = TextureImporterType.NormalMap;
                                break;
                            case TextureType.NonColor:
                                importer.sRGBTexture = false;
                                break;
                        }
                    }
                }
            };
#endif

            TextureFormat format = src.format;
            if (format == TextureFormat.RawFile)
            {
#if UNITY_EDITOR
                // write data to file and import
                string path = m_assetsFolder + "/" + src.name;
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
                            path = m_assetsFolder + "/" + src.name + ".png";
                            exported = TextureData.WriteToFile(path, EncodeToPNG(texture));
                            break;
                        }
                    case TextureFormat.Rf16:
                    case TextureFormat.RGf16:
                    case TextureFormat.RGBf16:
                    case TextureFormat.RGBAf16:
                        {
                            path = m_assetsFolder + "/" + src.name + ".exr";
                            exported = TextureData.WriteToFile(path, EncodeToEXR(texture, Texture2D.EXRFlags.CompressZIP));
                            break;
                        }
                    case TextureFormat.Rf32:
                    case TextureFormat.RGf32:
                    case TextureFormat.RGBf32:
                    case TextureFormat.RGBAf32:
                        {
                            path = m_assetsFolder + "/" + src.name + ".exr";
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
                TextureHolder dst = m_textureList.Find(a => a.id == id);
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
            return ImageConversion.EncodeToPNG(tex);
        }
        byte[] EncodeToEXR(Texture2D tex, Texture2D.EXRFlags flags)
        {
            return ImageConversion.EncodeToEXR(tex, flags);
        }

        private protected abstract void UpdateMaterialAssetV(MaterialData materialData);

        private protected void UpdateMaterialAssetByDefault(MaterialData src, ModelImporterSettings importerSettings)
        {
            int materialID = src.id;
            string materialName = src.name;

            MaterialHolder dst = m_materialList.Find(a => a.id == materialID);

            //if (invalid && creating materials is allowed)
            if ((dst == null || dst.material == null || dst.name != materialName) && importerSettings.CreateMaterials) {

                if (null == dst) {
                    dst = new MaterialHolder { id = materialID };
                    m_materialList.Add(dst);
                }
                Assert.IsNotNull(dst);

#if UNITY_EDITOR
                dst.material = SearchMaterialInEditor(importerSettings.MaterialSearchMode, materialName);
                if (null != dst.material) {
                    dst.ShouldApplyMaterialData = false;
                }
                else
#endif
                {
                    dst.material = CreateDefaultMaterial(src.shader);
                    dst.material.name = materialName;
                }
                m_needReassignMaterials = true;
            }

            if (dst == null || dst.material == null)
                return;

            dst.name = materialName;
            dst.index = src.index;
            dst.color = src.color;

            Material destMat = dst.material;
            if (importerSettings.CreateMaterials && dst.ShouldApplyMaterialData) {
                ApplyMaterialDataToMaterial(src, destMat, m_textureList);
            }

            if (onUpdateMaterial != null)
                onUpdateMaterial.Invoke(destMat, src);
        }

        //----------------------------------------------------------------------------------------------------------------------    

#if UNITY_EDITOR

        [CanBeNull]
        Material SearchMaterialInEditor(AssetSearchMode materialSearchMode, string materialName) {

            HashSet<string> materialPaths = null;
            string assetsFolder = GetAssetsFolder();
            switch (materialSearchMode) {
                case AssetSearchMode.LOCAL: {
                        if (Directory.Exists(assetsFolder)) {
                            materialPaths = AssetEditorUtility.FindAssetPaths("t:Material ", materialName, new string[] { assetsFolder });
                        }
                        break;
                    }

                case AssetSearchMode.RECURSIVE_UP: {
                        string nextFolder = assetsFolder;
                        while (!string.IsNullOrEmpty(nextFolder) && (null == materialPaths || materialPaths.Count <= 0)) {
                            if (Directory.Exists(nextFolder)) {
                                materialPaths = AssetEditorUtility.FindAssetPaths("t:Material ", materialName, new string[] { nextFolder }, false);
                            }

                            nextFolder = PathUtility.GetDirectoryName(nextFolder, 1);
                        }
                        break;
                    }
                case AssetSearchMode.EVERYWHERE: {
                        materialPaths = AssetEditorUtility.FindAssetPaths("t:Material ", materialName);
                        break;
                    }
            }

            if (null == materialPaths || materialPaths.Count <= 0)
                return null;

            Material candidate = null;

            foreach (string materialPath in materialPaths) {

                Material material = AssetDatabase.LoadAssetAtPath<Material>(materialPath);
                candidate = material;
                //if there are multiple candidates, prefer the editable one (= not a part of fbx etc)
                if (((int)material.hideFlags & (int)HideFlags.NotEditable) == 0)
                    return material;

            }
            Assert.IsNotNull(candidate);
            return candidate;
        }
#endif

        //----------------------------------------------------------------------------------------------------------------------    

        void ApplyMaterialDataToMaterial(MaterialData src, Material destMat,
            List<TextureHolder> textureHolders) {
            int numKeywords = src.numKeywords;
            for (int ki = 0; ki < numKeywords; ++ki) {
                MaterialKeywordData kw = src.GetKeyword(ki);
                if (kw.value)
                    destMat.EnableKeyword(kw.name);
                else
                    destMat.DisableKeyword(kw.name);
            }

            UpdateShader(destMat, src.shader);

            // Put all properties in a list so we can look them up more easily:
            int numProps           = src.numProperties;
            var materialProperties = new Dictionary<int, MaterialPropertyData>(numProps);

            for (int pi = 0; pi < numProps; ++pi) {
                MaterialPropertyData prop = src.GetProperty(pi);
                materialProperties.Add(prop.nameID, prop);
            }

            foreach (var prop in materialProperties) {
                ApplyMaterialProperty(destMat, textureHolders, prop.Value, prop.Key, materialProperties);
            }

            MapsBaker.BakeMaps(destMat, textureHolders, materialProperties);
        }

        private static bool HandleKeywords(Material destMat, List<TextureHolder> textureHolders,
            MaterialPropertyData prop, string keyword) {
            // If the texture exists, enable its keyword, otherwise disable it:
            if (prop.type == MaterialPropertyData.Type.Texture) {
                MaterialPropertyData.TextureRecord rec = prop.textureValue;
                Texture2D                          tex = FindTexture(rec.id, textureHolders);
                
                if (tex != null) {
                    destMat.EnableKeyword(keyword);
                    return true;
                }
            }

            destMat.DisableKeyword(keyword);

            return false;
        }

        private static Dictionary<int, int[]> synonymMap = new Dictionary<int, int[]> {
            { MeshSyncConstants._Color, new[] { MeshSyncConstants._BaseColor } },
            { MeshSyncConstants._MainTex, new[] { MeshSyncConstants._BaseMap, MeshSyncConstants._BaseColorMap } },
            { MeshSyncConstants._Glossiness, new[] { MeshSyncConstants._Smoothness, MeshSyncConstants._GlossMapScale } },
            { MeshSyncConstants._BumpMap, new[] { MeshSyncConstants._NormalMap } },
            { MeshSyncConstants._EmissionMap, new[] { MeshSyncConstants._EmissiveColorMap } },
            { MeshSyncConstants._ParallaxMap, new[] { MeshSyncConstants._HeightMap } },
            { MeshSyncConstants._BumpScale, new[] { MeshSyncConstants._NormalScale } }
        };

        private void ApplyMaterialProperty(Material destMat, 
            List<TextureHolder> textureHolders,
            MaterialPropertyData prop,
            int propNameID, 
            Dictionary<int, MaterialPropertyData> materialProperties) {
           
            // Shaders use different names to refer to the same maps, this map contains those synonyms so we can apply them to every shader easily:
            if (synonymMap.TryGetValue(propNameID, out var synonyms)) {
                foreach (var synonym in synonyms) {
                    ApplyMaterialProperty(destMat, textureHolders, prop, synonym, materialProperties);
                }
            }
            
            MaterialPropertyData.Type propType = prop.type;
            if (!destMat.HasProperty(propNameID))
                return;
            
            // Enable alpha test if there is a color texture so alpha clipping works:
            if (propNameID == MeshSyncConstants._BaseMap ||
                propNameID == MeshSyncConstants._MainTex || 
                propNameID == MeshSyncConstants._BaseColorMap) {
                bool hasAlpha = HandleKeywords(destMat, textureHolders, prop, MeshSyncConstants._ALPHATEST_ON);
                if (hasAlpha) {
                    destMat.SetOverrideTag(MeshSyncConstants.RenderType,MeshSyncConstants.TransparentCutout);
                }
#if AT_USE_HDRP
                destMat.SetFloat(MeshSyncConstants._AlphaCutoffEnable, hasAlpha ? 1 : 0);
                if (hasAlpha) {
                    destMat.renderQueue = (int)RenderQueue.AlphaTest + 25;
                    destMat.SetFloat(MeshSyncConstants._ZTestGBuffer, 3);
                }
#elif AT_USE_URP
                destMat.SetFloat(MeshSyncConstants._AlphaClip, hasAlpha ? 1 : 0);
#else 
                if (hasAlpha) {
                    destMat.SetFloat(MeshSyncConstants._Mode, 1);
                }
#endif
            }

            if (propNameID == MeshSyncConstants._EmissionColor) {
                if (destMat.globalIlluminationFlags == MaterialGlobalIlluminationFlags.EmissiveIsBlack) {
                    destMat.globalIlluminationFlags = MaterialGlobalIlluminationFlags.RealtimeEmissive;

                    HandleKeywords(destMat, textureHolders, prop, MeshSyncConstants._EMISSION);
                }
            }
            else if (propNameID == MeshSyncConstants._MetallicGlossMap) {
                HandleKeywords(destMat, textureHolders, prop, MeshSyncConstants._METALLICGLOSSMAP);
                HandleKeywords(destMat, textureHolders, prop, MeshSyncConstants._METALLICSPECGLOSSMAP);
            }
            else if (propNameID == MeshSyncConstants._BumpMap) {
                HandleKeywords(destMat, textureHolders, prop, MeshSyncConstants._NORMALMAP);
            }
            else if (propNameID == MeshSyncConstants._ParallaxMap) {
                HandleKeywords(destMat, textureHolders, prop, MeshSyncConstants._PARALLAXMAP);
            }
#if AT_USE_HDRP
            else if (propNameID == MeshSyncConstants._EmissiveColorMap) {
                Color baseEmissionColor = Color.white;

                if (materialProperties.TryGetValue(MeshSyncConstants._EmissionColor, out var emissionColorProp)) {
                    baseEmissionColor = emissionColorProp.vectorValue;
                }

                float emissionStrength = 0;
                if (materialProperties.TryGetValue(MeshSyncConstants._EmissionStrength, out var emissionStrengthProp)) {
                    emissionStrength = emissionStrengthProp.floatValue;
                }

                destMat.SetFloat(MeshSyncConstants._UseEmissiveIntensity, 1);
                destMat.SetFloat(MeshSyncConstants._EmissiveIntensityUnit, 0);
                destMat.SetFloat(MeshSyncConstants._EmissiveIntensity, emissionStrength);
                destMat.SetColor(MeshSyncConstants._EmissiveColorLDR, baseEmissionColor);
                destMat.SetColor(MeshSyncConstants._EmissiveColor, baseEmissionColor * emissionStrength);
            }
            else if (propNameID == MeshSyncConstants._HeightMap) {
                if (prop.type == MaterialPropertyData.Type.Texture) {
                    MaterialPropertyData.TextureRecord rec = prop.textureValue;
                    Texture2D                          tex = FindTexture(rec.id, textureHolders);
                    if (tex != null) {
                        destMat.EnableKeyword(MeshSyncConstants._HEIGHTMAP);

                        // If there is no displacement mode set, set it to vertex displacement:
                        if (!destMat.IsKeywordEnabled(MeshSyncConstants._PIXEL_DISPLACEMENT)) {
                            destMat.EnableKeyword(MeshSyncConstants._VERTEX_DISPLACEMENT);
                            destMat.SetInt(MeshSyncConstants._DisplacementMode, 1);
                        }

                        // Fallback value in case the node setup in blender does not specify the height scale.
                        // Normally this is not used because the value comes from the displacement node:
                        float scale = 10;
                        if (materialProperties.TryGetValue(MeshSyncConstants._Parallax, out var heightScaleProp)) {
                            scale = heightScaleProp.floatValue;
                        }

                        destMat.SetFloat(MeshSyncConstants._HeightMin, -scale);
                        destMat.SetFloat(MeshSyncConstants._HeightMax, scale);

                        // Amplitude is the difference between min height and max height: 
                        float amplitude = scale * 2;
                        
                        // Convert from meters to centimeters:
                        destMat.SetFloat(MeshSyncConstants._HeightAmplitude, amplitude * 0.01f);
                    }
                }
            }
            else if (propNameID == MeshSyncConstants._CoatMaskMap) {
                HandleKeywords(destMat, textureHolders, prop, MeshSyncConstants._MATERIAL_FEATURE_CLEAR_COAT);
            }
            else if (propNameID == MeshSyncConstants._CoatMask) {
                destMat.EnableKeyword(MeshSyncConstants._MATERIAL_FEATURE_CLEAR_COAT);
            }
#endif

            int len = prop.arrayLength;
            switch (propType) {
                case MaterialPropertyData.Type.Int:
                    destMat.SetInt(propNameID, prop.intValue);
                    break;
                case MaterialPropertyData.Type.Float:
                    if (len == 1)
                        destMat.SetFloat(propNameID, prop.floatValue);
                    else
                        destMat.SetFloatArray(propNameID, prop.floatArray);
                    break;
                case MaterialPropertyData.Type.Vector:
                    if (len == 1)
                        destMat.SetVector(propNameID, prop.vectorValue);
                    else
                        destMat.SetVectorArray(propNameID, prop.vectorArray);
                    break;
                case MaterialPropertyData.Type.Matrix:
                    if (len == 1)
                        destMat.SetMatrix(propNameID, prop.matrixValue);
                    else
                        destMat.SetMatrixArray(propNameID, prop.matrixArray);
                    break;
                case MaterialPropertyData.Type.Texture: {
                    MaterialPropertyData.TextureRecord rec = prop.textureValue;
                    Texture2D                          tex = FindTexture(rec.id, textureHolders);
                    // Allow setting of null textures to clear them:
                    destMat.SetTextureAndReleaseExistingRenderTextures(propNameID, tex);
                    if (rec.hasScaleOffset) {
                        destMat.SetTextureScale(propNameID, rec.scale);
                        destMat.SetTextureOffset(propNameID, rec.offset);
                    }
                }
                    break;
                case MaterialPropertyData.Type.String:
                    // Not used at the moment but would be: prop.stringValue;
                    break;
                default: break;
            }
        }

        //----------------------------------------------------------------------------------------------------------------------

        EntityRecord UpdateMeshEntity(MeshData data, MeshSyncPlayerConfig config) {
            if (!config.SyncMeshes)
                return null;

            TransformData dtrans = data.transform;
            MeshDataFlags dflags = data.dataFlags;
            EntityRecord rec = UpdateTransformEntity(dtrans, config);
            if (rec == null || dflags.unchanged)
                return null;

            if (!string.IsNullOrEmpty(rec.reference))
            {
                // references will be resolved later in UpdateReference()
                return null;
            }

            string path = dtrans.path;
            GameObject go = rec.go;
            bool activeInHierarchy = go.activeInHierarchy;
            if (!activeInHierarchy && !dflags.hasPoints)
                return null;

            return UpdateMeshEntity(data, config, rec);
        }

        private EntityRecord UpdateMeshEntity(MeshData data, MeshSyncPlayerConfig config, EntityRecord rec)
        {
            TransformData dtrans = data.transform;
            MeshDataFlags dflags = data.dataFlags;
            GameObject go = rec.go;
            Transform trans = go.transform;

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
                        if (!m_saveAssetsInScene)
                            rec.mesh.hideFlags = HideFlags.DontSaveInEditor;
                        rec.mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
                    }

                    UpdateMeshEntity(ref rec.mesh, data);
                }

                meshUpdated = true;
            }
            else if (rec.mesh != null) {
                rec.mesh.Clear();
            }

            if (dflags.hasBones || dflags.hasBlendshapes)
            {
                SkinnedMeshRenderer smr = rec.skinnedMeshRenderer;
                if (smr == null)
                {
                    materialsUpdated = true;
                    smr = rec.skinnedMeshRenderer = Misc.GetOrAddComponent<SkinnedMeshRenderer>(trans.gameObject);
                    rec.DestroyMeshRendererAndFilter();
                }

                rec.smrUpdated = true;
                if (config.SyncVisibility && dtrans.dataFlags.hasVisibility)
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
                    smr.localBounds = (null != rec.mesh) ? rec.mesh.bounds : new Bounds();
                    smr.updateWhenOffscreen = false;
                }

                // update blendshape weights
                if (dflags.hasBlendshapes)
                {
                    int meshBlendShapeCount = (null != rec.mesh) ? rec.mesh.blendShapeCount : 0;
                    int numBlendShapes = Math.Min(data.numBlendShapes, meshBlendShapeCount);
                    for (int bi = 0; bi < numBlendShapes; ++bi)
                    {
                        BlendShapeData bsd = data.GetBlendShapeData(bi);
                        smr.SetBlendShapeWeight(bi, bsd.weight);
                    }
                }
                
#if AT_USE_HDRP
                UpdateRayTracingModeIfNecessary(smr);
#endif
            }
            else if (meshUpdated)
            {
                MeshFilter mf = rec.meshFilter;
                MeshRenderer mr = rec.meshRenderer;

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

                if (config.SyncVisibility && dtrans.dataFlags.hasVisibility)
                    mr.enabled = data.transform.visibility.visibleInRender;
                mf.sharedMesh = rec.mesh;
                rec.smrEnabled = false;
                
#if AT_USE_HDRP
                UpdateRayTracingModeIfNecessary(mr);
#endif

            }

            if (meshUpdated)
            {
                MeshCollider collider = config.UpdateMeshColliders ? trans.GetComponent<MeshCollider>() : null;
                if (collider != null &&
                    (collider.sharedMesh == null || collider.sharedMesh == rec.mesh))
                {
                    collider.sharedMesh = rec.mesh;
                }
            }

            // assign materials if needed
            if (materialsUpdated)
                AssignMaterials(rec, recordUndo: false);

#if AT_USE_PROBUILDER
            if (UseProBuilder)
            {
                if (rec.mesh != null && (meshUpdated || materialsUpdated))
                {
                    //Debug.LogError("Rebuilding");
                    ProBuilderBeforeRebuild?.Invoke();

                    rec.proBuilderMeshFilter = Misc.GetOrAddComponent<UnityEngine.ProBuilder.ProBuilderMesh>(trans.gameObject);

                    Material[] sharedMaterials = null;
                    if (rec.meshRenderer != null) {
                        sharedMaterials = rec.meshRenderer.sharedMaterials;
                    }

                    var importer = new UnityEngine.ProBuilder.MeshOperations.MeshImporter(rec.mesh, sharedMaterials, rec.proBuilderMeshFilter);
                    // Disable quads, it is much slower:
                    importer.Import(new UnityEngine.ProBuilder.MeshOperations.MeshImportSettings() { quads = false });

                    // To refresh the internal probuilder mesh:
                    rec.proBuilderMeshFilter.ToMesh();
                    rec.proBuilderMeshFilter.Refresh();

                    ProBuilderAfterRebuild?.Invoke();
                }
            }
            else
            {
                if (rec.proBuilderMeshFilter != null)
                {
                    DestroyImmediate(rec.proBuilderMeshFilter);
                    rec.proBuilderMeshFilter = null;
                }
            }
#endif
            return rec;
        }

#if AT_USE_PROBUILDER
        internal static Action ProBuilderBeforeRebuild;
        internal static Action ProBuilderAfterRebuild;
#endif
      
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
        
#if AT_USE_HDRP

        void UpdatePathTracingState() {
            m_pathTracingExists = false;
            if (!HDRPUtility.IsRayTracingActive()) 
                return;
            
            SceneComponents<Volume> sceneVolumes = SceneComponents<Volume>.GetInstance();
            sceneVolumes.Update();
            m_pathTracingExists = HDRPUtility.IsPathTracingActive(sceneVolumes.GetCachedComponents());
        }
        
        void UpdateRayTracingModeIfNecessary(Renderer r) {
            if (!m_pathTracingExists) 
                return;
            
            r.rayTracingMode         = UnityEngine.Experimental.Rendering.RayTracingMode.DynamicGeometry;
            m_needToResetPathTracing = true;
        }
#endif
        

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

        void UpdateMeshEntity(ref Mesh mesh, MeshData data)
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

            int numPoints = data.numPoints;
            MeshDataFlags dataFlags = data.dataFlags;
            if (dataFlags.hasPoints) {
                m_tmpV3.Resize(numPoints);
                data.ReadPoints(m_tmpV3);
                mesh.SetVertices(m_tmpV3.List);
            }
            if (dataFlags.hasNormals) {
                m_tmpV3.Resize(numPoints);
                data.ReadNormals(m_tmpV3);
                mesh.SetNormals(m_tmpV3.List);
            }
            if (dataFlags.hasTangents) {
                m_tmpV4.Resize(numPoints);
                data.ReadTangents(m_tmpV4);
                mesh.SetTangents(m_tmpV4.List);
            }

            for (int i = 0; i < CoreAPIConstants.MAX_UV; ++i) {
                if (dataFlags.HasUV(i)) {
                    m_tmpV2.Resize(numPoints);
                    data.ReadUV(m_tmpV2, i);
                    mesh.SetUVs(i, m_tmpV2.List);
                }
            }
            if (dataFlags.hasColors) {
                m_tmpC.Resize(numPoints);
                data.ReadColors(m_tmpC);
                mesh.SetColors(m_tmpC.List);
            }
            if (dataFlags.hasBones) {
                mesh.bindposes = data.bindposes;

                {
                    // bonesPerVertex + weights1
                    NativeArray<byte> bonesPerVertex = new NativeArray<byte>(numPoints, Allocator.Temp);
                    NativeArray<BoneWeight1> weights1 = new NativeArray<BoneWeight1>(data.numBoneWeights, Allocator.Temp);
                    data.ReadBoneCounts(Misc.ForceGetPointer(ref bonesPerVertex));
                    data.ReadBoneWeightsV(Misc.ForceGetPointer(ref weights1));
                    mesh.SetBoneWeights(bonesPerVertex, weights1);
                    bonesPerVertex.Dispose();
                    weights1.Dispose();
                }
            }
            if (dataFlags.hasIndices && !keepIndices) {
                int subMeshCount = data.numSubmeshes;
                mesh.subMeshCount = subMeshCount;
                for (int smi = 0; smi < subMeshCount; ++smi) {
                    SubmeshData submesh = data.GetSubmesh(smi);
                    SubmeshData.Topology topology = submesh.topology;

                    m_tmpI.Resize(submesh.numIndices);
                    submesh.ReadIndices(data, m_tmpI);

                    if (topology == SubmeshData.Topology.Triangles) {
                        mesh.SetTriangles(m_tmpI.List, smi, false);
                    } else {
                        MeshTopology mt = MeshTopology.Points;
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
            if (dataFlags.hasBlendshapes) {
                PinnedList<Vector3> tmpBSP = new PinnedList<Vector3>(numPoints);
                PinnedList<Vector3> tmpBSN = new PinnedList<Vector3>(numPoints);
                PinnedList<Vector3> tmpBST = new PinnedList<Vector3>(numPoints);

                int numBlendShapes = data.numBlendShapes;
                for (int bi = 0; bi < numBlendShapes; ++bi) {
                    BlendShapeData bsd = data.GetBlendShapeData(bi);
                    string name = bsd.name;
                    float numFrames = bsd.numFrames;
                    for (int fi = 0; fi < numFrames; ++fi) {
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

        //----------------------------------------------------------------------------------------------------------------------        
        EntityRecord UpdatePointsEntity(PointsData data, MeshSyncPlayerConfig config)
        {

            TransformData dtrans = data.transform;
            PointsDataFlags dflags = data.dataFlags;
            EntityRecord rec = UpdateTransformEntity(dtrans, config);
            if (rec == null || dflags.unchanged)
                return null;

            // reference (source mesh) will be resolved in UpdateReference()

            string path = dtrans.path;
            GameObject go = rec.go;

            PointCache dst = rec.pointCache;
            if (dst == null) {
                rec.pointCacheRenderer = Misc.GetOrAddComponent<PointCacheRenderer>(go);
                dst = rec.pointCache = Misc.GetOrAddComponent<PointCache>(go);
            }
            dst.Clear();

            int num = data.numPoints;
            dst.bounds = data.bounds;
            if (dflags.hasPoints) {
                dst.points = new Vector3[num];
                data.ReadPoints(dst.points);
            }
            if (dflags.hasRotations) {
                dst.rotations = new Quaternion[num];
                data.ReadRotations(dst.rotations);
            }
            if (dflags.hasScales) {
                dst.scales = new Vector3[num];
                data.ReadScales(dst.scales);
            }
            return rec;
        }

        EntityRecord UpdateTransformEntity(TransformData data, MeshSyncPlayerConfig config)
        {
            string path = data.path;
            int hostID = data.hostID;
            if (path.Length == 0)
                return null;

            Transform trans = null;
            EntityRecord rec = null;
            if (hostID != Lib.invalidID) {
                if (m_hostObjects.TryGetValue(hostID, out rec)) {
                    if (rec.go == null) {
                        m_hostObjects.Remove(hostID);
                        rec = null;
                    }
                }
                if (rec == null)
                    return null;
            } else {
                if (m_clientObjects.TryGetValue(path, out rec)) {
                    if (rec.go == null)
                    {
                        m_clientObjects.Remove(path);
                        rec = null;
                    }
                }

                if (rec == null) {
                    trans = FilmInternalUtilities.GameObjectUtility.FindOrCreateByPath(m_rootObject, path, false);
                    rec = new EntityRecord {
                        go = trans.gameObject,
                        trans = trans,
                        recved = true,
                    };
                    m_clientObjects.Add(path, rec);
                }
            }

            return UpdateTransformEntity(data, config, rec);
        }
        void SetVis(Behaviour c, VisibilityFlags visibility)
        {
            if (c != null)
            {
                c.enabled = visibility.visibleInRender;
            }
        }

        void SetVis(Renderer c, VisibilityFlags visibility)
        {
            if (c != null)
            {
                c.enabled = visibility.visibleInRender;
            }
        }

        private EntityRecord UpdateTransformEntity(TransformData data, MeshSyncPlayerConfig config, EntityRecord rec)
        {
            var trans = rec.trans;
            if (trans == null)
                trans = rec.trans = rec.go.transform;

            TransformDataFlags dflags = data.dataFlags;
            if (!dflags.unchanged)
            {
                VisibilityFlags visibility = data.visibility;
                rec.index = data.index;
                rec.dataType = data.entityType;

                // sync TRS
                if (config.SyncTransform)
                {
                    if (dflags.hasPosition)
                        trans.localPosition = data.position;
                    if (dflags.hasRotation)
                        trans.localRotation = data.rotation;
                    if (dflags.hasScale)
                        trans.localScale = data.scale;
                }

                // visibility
                if (config.SyncVisibility && dflags.hasVisibility)
                {
                    trans.gameObject.SetActive(visibility.active);

                    if (!visibility.active)
                    {
                        // If the parent is invisible because of layer visibility, make the
                        // child active inside the parent so enabling the parent also enables
                        // the child. This is important so instances work correctly.
                        var parentPath = data.path.Substring(0, data.path.LastIndexOf('/'));

                        if (m_clientObjects.TryGetValue(parentPath, out var parentEntity))
                        {
                            if (parentEntity.hasVisibility &&
                                !parentEntity.visibility.active)
                            {
                                trans.gameObject.SetActive(visibility.visibleInRender);
                            }
                        }
                    }

                    SetVis(rec.meshRenderer, visibility);
                    SetVis(rec.camera, visibility);
                    SetVis(rec.skinnedMeshRenderer, visibility);
                }

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

        EntityRecord UpdateCameraEntity(CameraData data, MeshSyncPlayerConfig config) {
            ComponentSyncSettings cameraSyncSettings = config.GetComponentSyncSettings(MeshSyncPlayerConfig.SYNC_CAMERA);
            if (!cameraSyncSettings.CanCreate)
                return null;

            TransformData dtrans = data.transform;
            CameraDataFlags dflags = data.dataFlags;
            EntityRecord rec = UpdateTransformEntity(dtrans, config);
            if (rec == null || dflags.unchanged)
                return null;

            Camera cam = rec.camera;
            if (cam == null)
                cam = rec.camera = Misc.GetOrAddComponent<Camera>(rec.go);

            if (!cameraSyncSettings.CanUpdate)
                return null;

            if (config.SyncVisibility && dtrans.dataFlags.hasVisibility)
                cam.enabled = dtrans.visibility.visibleInRender;

            cam.orthographic = data.orthographic;

            // use physical camera params if available
            if (config.IsPhysicalCameraParamsUsed() && dflags.hasFocalLength && dflags.hasSensorSize)
            {
                cam.usePhysicalProperties = true;
                cam.focalLength = data.focalLength;
                cam.sensorSize = data.sensorSize;
                cam.lensShift = data.lensShift;
                //todo: gate fit support
            }
            else
            {
                if (dflags.hasFov)
                    cam.fieldOfView = data.fov;
            }

            if (dflags.hasNearPlane && dflags.hasFarPlane) {
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

        EntityRecord UpdateLightEntity(LightData data, MeshSyncPlayerConfig config) {
            ComponentSyncSettings syncLightSettings = config.GetComponentSyncSettings(MeshSyncPlayerConfig.SYNC_LIGHTS);
            if (!syncLightSettings.CanCreate)
                return null;

            TransformData dtrans = data.transform;
            LightDataFlags dflags = data.dataFlags;
            EntityRecord rec = UpdateTransformEntity(dtrans, config);
            if (rec == null || dflags.unchanged)
                return null;

            rec.SetLight(data, config.SyncVisibility, syncLightSettings.CanUpdate);
            return rec;
        }

        void UpdateReference(EntityRecord dst, EntityRecord src, MeshSyncPlayerConfig config)
        {
            if (src.dataType == EntityType.Unknown)
            {
                Debug.LogError("MeshSync: should not be here!");
                return;
            }

            GameObject dstgo = dst.go;
            GameObject srcgo = src.go;
            if (src.dataType == EntityType.Camera) {
                Camera srccam = src.camera;
                if (srccam != null) {
                    Camera dstcam = dst.camera;
                    if (dstcam == null)
                        dstcam = dst.camera = Misc.GetOrAddComponent<Camera>(dstgo);
                    if (config.SyncVisibility && dst.hasVisibility)
                        dstcam.enabled = dst.visibility.visibleInRender;
                    dstcam.enabled = srccam.enabled;
                    dstcam.orthographic = srccam.orthographic;
                    dstcam.fieldOfView = srccam.fieldOfView;
                    dstcam.nearClipPlane = srccam.nearClipPlane;
                    dstcam.farClipPlane = srccam.farClipPlane;
                }
            } else if (src.dataType == EntityType.Light) {
                dst.SetLight(src, config.SyncVisibility);
            }
            else if (src.dataType == EntityType.Mesh)
            {
                Mesh mesh = src.mesh;
                MeshRenderer srcmr = src.meshRenderer;
                SkinnedMeshRenderer srcsmr = src.skinnedMeshRenderer;

                if (mesh != null)
                {
                    PointCacheRenderer dstpcr = dst.pointCacheRenderer;
                    if (dstpcr != null) {
                        if (config.SyncVisibility && dst.hasVisibility)
                            dstpcr.enabled = dst.visibility.visibleInRender;
                        dstpcr.sharedMesh = mesh;

                        if (dstpcr.sharedMaterials == null || dstpcr.sharedMaterials.Length == 0) {
                            Material[] materials = null;
                            if (srcmr != null)
                                materials = srcmr.sharedMaterials;
                            else if (srcsmr != null)
                                materials = srcsmr.sharedMaterials;

                            if (materials != null) {
                                foreach (Material m in materials)
                                    m.enableInstancing = true;
                                dstpcr.sharedMaterials = materials;
                            }
                        }
                    }
                    else if (srcmr != null)
                    {
                        MeshRenderer dstmr = dst.meshRenderer;
                        MeshFilter dstmf = dst.meshFilter;
                        if (dstmr == null) {
                            dstmr = dst.meshRenderer = Misc.GetOrAddComponent<MeshRenderer>(dstgo);
                            dstmf = dst.meshFilter = Misc.GetOrAddComponent<MeshFilter>(dstgo);
                        }

                        if (config.SyncVisibility && dst.hasVisibility)
                            dstmr.enabled = dst.visibility.visibleInRender;
                        dstmf.sharedMesh = mesh;
                        dstmr.sharedMaterials = srcmr.sharedMaterials;
                    } else if (srcsmr != null) {
                        SkinnedMeshRenderer dstsmr = dst.skinnedMeshRenderer;
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

                        if (config.SyncVisibility && dst.hasVisibility)
                            dstsmr.enabled = dst.visibility.visibleInRender;
                        else
                            dstsmr.enabled = oldEnabled;
                    }

                    // handle mesh collider
                    if (config.UpdateMeshColliders) {
                        MeshCollider srcmc = srcgo.GetComponent<MeshCollider>();
                        if (srcmc != null && srcmc.sharedMesh == mesh) {
                            MeshCollider dstmc = Misc.GetOrAddComponent<MeshCollider>(dstgo);
                            dstmc.enabled = srcmc.enabled;
                            dstmc.isTrigger = srcmc.isTrigger;
                            dstmc.sharedMaterial = srcmc.sharedMaterial;
                            dstmc.sharedMesh = mesh;
                            dstmc.convex = srcmc.convex;
                            dstmc.cookingOptions = srcmc.cookingOptions;
                        }
                    }
                }
            } else if (src.dataType == EntityType.Points) {
                PointCacheRenderer srcpcr = src.pointCacheRenderer;
                if (srcpcr != null) {
                    PointCacheRenderer dstpcr = dst.pointCacheRenderer;
                    dstpcr.sharedMesh = srcpcr.sharedMesh;

                    Material[] materials = srcpcr.sharedMaterials;
                    for (int i = 0; i < materials.Length; ++i)
                        materials[i].enableInstancing = true;
                    dstpcr.sharedMaterials = materials;
                }
            }
        }

        EntityRecord UpdateInstancedEntity(TransformData data)
        {
            var config = GetConfigV();

            if (!m_clientInstancedEntities.TryGetValue(data.path, out EntityRecord rec) || rec.go == null)
            {
                var trans =
                    FilmInternalUtilities.GameObjectUtility.FindOrCreateByPath(m_rootObject, data.path, false);

                rec = new EntityRecord {
                    go = trans.gameObject,
                    trans = trans,
                    recved = true,
                };

                m_clientInstancedEntities[data.path] = rec;
            }

            UpdateTransformEntity(data, config, rec);
            UpdateMeshEntity((MeshData)data, config, rec);

            m_clientInstancedEntities[data.path] = rec;

            // If there is a instance info record that references the new object
            if (m_clientInstances.TryGetValue(data.path, out InstanceInfoRecord infoRecord)) {
                // Update the renderer reference
                infoRecord.go = rec.go;
                var renderer = infoRecord.renderer;
                if (renderer != null) {
                    renderer.UpdateReference(rec.go);
                }
            }

            return rec;
        }

        HashSet<string> instanceNames = new HashSet<string>();
        InstanceInfoRecord UpdateInstanceInfo(InstanceInfoData data)
        {
            var path = data.path;

            InstanceInfoRecord infoRecord;
            if (!m_clientInstances.TryGetValue(path, out infoRecord))
            {
                infoRecord = new InstanceInfoRecord();
                m_clientInstances.Add(path, infoRecord);
            }

            EntityRecord instancedEntityRecord;
            // Look for reference in client instanced objects
            if (m_clientInstancedEntities.TryGetValue(path, out instancedEntityRecord))
            {
                infoRecord.go = instancedEntityRecord.go;
            }
            else if (m_clientObjects.TryGetValue(path, out instancedEntityRecord))
            {
                infoRecord.go = instancedEntityRecord.go;
            }
            else
            {
                throw new Exception($"[MeshSync] No instanced entity found for path {data.path}");
            }

            // Find parent for this instance:
            GameObject instanceRendererParent;
            if (m_clientObjects.TryGetValue(data.parentPath, out EntityRecord parentRecord))
            {
                instanceRendererParent = parentRecord.go;
            }
            else
            {
                instanceRendererParent = m_rootObject.gameObject;

                Debug.LogWarningFormat(
                    "[MeshSync] No entity found for parent path {0}, using root as parent", data.parentPath);
            }

            // Anything other than a mesh needs to use copies if we're in instance renderer mode:
            var instanceHandlingToUse = InstanceHandling;
            if (instanceHandlingToUse == InstanceHandlingType.InstanceRenderer &&
                instancedEntityRecord.dataType != EntityType.Mesh) {
                instanceHandlingToUse = InstanceHandlingType.Copies;
            }

            switch (instanceHandlingToUse)
            {
                case InstanceHandlingType.InstanceRenderer:
                    UpdateInstanceInfo_InstanceRenderer(data, infoRecord, instanceRendererParent);
                    break;
                case InstanceHandlingType.Copies:
                case InstanceHandlingType.Prefabs:
                    UpdateInstanceInfo_CopiesAndPrefabs(data, infoRecord, instancedEntityRecord, instanceRendererParent);
                    break;
                default:
                    break;
            }

            return infoRecord;
        }

        private void UpdateInstanceInfo_CopiesAndPrefabs(
            InstanceInfoData data,
            InstanceInfoRecord infoRecord,
            EntityRecord instancedEntityRecord,
            GameObject instanceRendererParent) {
            // Make sure the other renderer is removed if this was not a copy or prefab before:
            if (infoRecord.renderer != null) {
                DestroyImmediate(infoRecord.renderer);
                infoRecord.renderer = null;
            }

            infoRecord.DeleteInstanceObjects();

            instanceNames.Add(infoRecord.go.name);

            MeshSyncPlayerConfig config = GetConfigV();

            if (infoRecord.go.transform.parent == null ||
                !instanceNames.Contains(infoRecord.go.transform.parent.name)) {
                bool visible = instancedEntityRecord.visibility.visibleInRender;

                GameObject instanceObjectOriginal;
                if (InstanceHandling == InstanceHandlingType.Prefabs) {
                    instanceObjectOriginal = GetOrCreatePrefab(data, infoRecord);
                }
                else {
                    instanceObjectOriginal = infoRecord.go;
                }

                if (instanceObjectOriginal == null) {
                    return;
                }

                foreach (var mat in data.transforms) {
                    GameObject instancedCopy;
                    if (InstanceHandling == InstanceHandlingType.Prefabs) {
#if UNITY_EDITOR
                        instancedCopy = (GameObject)PrefabUtility.InstantiatePrefab(instanceObjectOriginal,
                            instanceRendererParent.transform);
#else
                        instancedCopy = Instantiate(instanceObjectOriginal, instanceRendererParent.transform);
#endif
                    }
                    else {
                        instancedCopy = Instantiate(instanceObjectOriginal, instanceRendererParent.transform);
                    }

                    SetInstanceTransform(instancedCopy, instanceRendererParent, mat);

                    if (config.SyncVisibility &&
                        instancedEntityRecord.hasVisibility) {
                        instancedCopy.SetActive(visible);
                    }

                    infoRecord.instanceObjects.Add(instancedCopy);
                }
            }
        }

        private static void SetInstanceTransform(GameObject instancedCopy, GameObject parent, Matrix4x4 mat) {
            Transform objTransform = instancedCopy.transform;
            
            mat = parent.transform.localToWorldMatrix * mat;

            objTransform.localScale = mat.lossyScale;
            objTransform.position   = mat.MultiplyPoint(Vector3.zero);

            // Calculate rotation here to avoid gimbal lock issue:
            Vector3 forward;
            forward.x = mat.m02;
            forward.y = mat.m12;
            forward.z = mat.m22;

            Vector3 upwards;
            upwards.x = mat.m01;
            upwards.y = mat.m11;
            upwards.z = mat.m21;

            objTransform.rotation = Quaternion.LookRotation(forward, upwards);

#if DEBUG
            var localMatrix = objTransform.localToWorldMatrix;
            for (int x = 0; x < 3; x++) {
                for (int y = 0; y < 3; y++) {
                    Debug.Assert( Math.Abs(localMatrix[x, y] - mat[x, y]) < 0.01f, "Matrices don't match!");
                }
            }
#endif
        }

        private GameObject GetOrCreatePrefab(InstanceInfoData data, InstanceInfoRecord infoRecord) {
            GameObject instanceObject = null;

            // Look for existing prefab and make sure the object exists:
            if (m_prefabDict.TryGetValue(data.path, out var existingPrefab)) {
                instanceObject = existingPrefab.prefab;
                if (instanceObject == null) {
                    m_prefabDict.Remove(data.path);
                }
            }

            // Create it if it doesn't exist:
            if (instanceObject == null) {
#if UNITY_EDITOR // Cannot create prefabs when not in editor.
                ExportMeshes();
                ExportMaterials();

                var pathWithoutSlash = data.path.Trim('/');
                var prefabLocation = Path.Combine(m_assetsFolder, Misc.SanitizeFileName(pathWithoutSlash) + ".prefab");

                bool wasActive = infoRecord.go.activeSelf;
                infoRecord.go.SetActive(true);

                var prefab = PrefabUtility.SaveAsPrefabAssetAndConnect(infoRecord.go, prefabLocation,
                    InteractionMode.AutomatedAction);

                AssetDatabase.SaveAssets();

                m_prefabDict[data.path] = new PrefabHolder { name = data.path, prefab = prefab };

                instanceObject = prefab;

                infoRecord.go.SetActive(wasActive);
#endif
            }

            return instanceObject;
        }

        private static void UpdateInstanceInfo_InstanceRenderer(InstanceInfoData data, InstanceInfoRecord infoRecord,
            GameObject instanceRendererParent) {
            infoRecord.DeleteInstanceObjects();

            if (instanceRendererParent != null && infoRecord.renderer == null) {
                infoRecord.renderer = instanceRendererParent.AddComponent<MeshSyncInstanceRenderer>();
            }

            infoRecord.renderer.UpdateAll(data.transforms, infoRecord.go);
        }

        void UpdateConstraint(ConstraintData data)
        {
            Transform trans = FilmInternalUtilities.GameObjectUtility.FindOrCreateByPath(m_rootObject, data.path, false);
            if (trans == null)
                return;

            Action<IConstraint> basicSetup = (c) => {
                int ns = data.numSources;
                while (c.sourceCount < ns)
                    c.AddSource(new ConstraintSource());
                for (int si = 0; si < ns; ++si) {
                    ConstraintSource s = c.GetSource(si);
                    s.sourceTransform = FilmInternalUtilities.GameObjectUtility.FindOrCreateByPath(m_rootObject, data.GetSourcePath(si), false);
                }
            };

            switch (data.type)
            {
                case ConstraintData.ConstraintType.Aim:
                    {
                        AimConstraint c = Misc.GetOrAddComponent<AimConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Parent:
                    {
                        ParentConstraint c = Misc.GetOrAddComponent<ParentConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Position:
                    {
                        PositionConstraint c = Misc.GetOrAddComponent<PositionConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Rotation:
                    {
                        RotationConstraint c = Misc.GetOrAddComponent<RotationConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                case ConstraintData.ConstraintType.Scale:
                    {
                        ScaleConstraint c = Misc.GetOrAddComponent<ScaleConstraint>(trans.gameObject);
                        basicSetup(c);
                        break;
                    }
                default:
                    break;
            }
        }

        //----------------------------------------------------------------------------------------------------------------------   

        void UpdateAnimationAsset(AnimationClipData clipData, MeshSyncPlayerConfig config) {
            MakeSureAssetDirectoryExists();

#if UNITY_EDITOR

            clipData.Convert((InterpolationMode)config.AnimationInterpolation);
            if (config.KeyframeReduction)
                clipData.KeyframeReduction(config.ReductionThreshold, config.ReductionEraseFlatCurves);

            //float start = Time.realtimeSinceStartup;

            Dictionary<GameObject, AnimationClip> animClipCache = new Dictionary<GameObject, AnimationClip>();

            int numAnimations = clipData.numAnimations;
            for (int ai = 0; ai < numAnimations; ++ai)
            {
                AnimationData data = clipData.GetAnimation(ai);

                string path = data.path;
                EntityRecord rec = null;
                m_clientObjects.TryGetValue(path, out rec);

                Transform target = null;
                if (rec != null)
                    target = rec.trans;
                if (target == null)
                {
                    target = FilmInternalUtilities.GameObjectUtility.FindOrCreateByPath(m_rootObject, path, false);
                    if (target == null)
                        return;
                }

                Transform root = target;
                while (root.parent != null && root.parent != m_rootObject)
                    root = root.parent;

                Animator animator = Misc.GetOrAddComponent<Animator>(root.gameObject);

                // get or create animation clip
                AnimationClip clip = null;
                if (!animClipCache.TryGetValue(root.gameObject, out clip)) {
                    if (animator.runtimeAnimatorController != null) {
                        AnimationClip[] clips = animator.runtimeAnimatorController.animationClips;
                        if (clips != null && clips.Length > 0) {
                            // note: this is extremely slow. animClipCache exists to cache the result and avoid frequent call.
                            AnimationClip tmp = animator.runtimeAnimatorController.animationClips[0];
                            if (tmp != null)
                            {
                                clip = tmp;
                                animClipCache[root.gameObject] = tmp;
                            }
                        }
                    }
                    if (clip == null) {
                        clip = new AnimationClip();
                        string clipName = clipData.name;
                        if (clipName.Length > 0)
                            clipName = root.name + "_" + clipName;
                        else
                            clipName = root.name;

                        string dstPath = m_assetsFolder + "/" + Misc.SanitizeFileName(clipName) + ".anim";
                        clip = Misc.OverwriteOrCreateAsset(clip, dstPath);
                        animator.runtimeAnimatorController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPathWithClip(dstPath + ".controller", clip);
                        animClipCache[root.gameObject] = clip;
                    }
                    clip.frameRate = clipData.frameRate;
                }

                string animPath = path.Replace("/" + root.name, "");
                if (animPath.Length > 0 && animPath[0] == '/')
                    animPath = animPath.Remove(0, 1);

                // get animation curves
                AnimationImportContext ctx = new AnimationImportContext() {
                    clip = clip,
                    root = root.gameObject,
                    target = target.gameObject,
                    path = animPath,
                    enableVisibility = config.SyncVisibility,
                    usePhysicalCameraParams = config.IsPhysicalCameraParamsUsed(),
                };
                if (rec != null)
                {
                    if (rec.meshRenderer != null) ctx.mainComponentType = typeof(MeshRenderer);
                    else if (rec.skinnedMeshRenderer != null) ctx.mainComponentType = typeof(SkinnedMeshRenderer);
                }
                data.ExportToClip(ctx);
            }

            //Debug.Log("UpdateAnimationAsset() " + (Time.realtimeSinceStartup - start) + " sec");

            // fire event
            if (onUpdateAnimation != null)
                foreach (KeyValuePair<GameObject, AnimationClip> kvp in animClipCache)
                    onUpdateAnimation.Invoke(kvp.Value, clipData);
#endif
        }

        private void ReassignMaterials(bool recordUndo) {
            foreach (KeyValuePair<string, EntityRecord> rec in m_clientObjects)
                AssignMaterials(rec.Value, recordUndo);
            foreach (KeyValuePair<int, EntityRecord> rec in m_hostObjects)
                AssignMaterials(rec.Value, recordUndo);
        }

        private void AssignMaterials(EntityRecord rec, bool recordUndo) {
            if (rec.go == null || rec.mesh == null || rec.materialIDs == null)
                return;

            Renderer r = rec.meshRenderer != null ? (Renderer)rec.meshRenderer : (Renderer)rec.skinnedMeshRenderer;
            if (r == null)
                return;

            bool changed = false;
            int materialCount = rec.materialIDs.Length;
            Material[] materials = new Material[materialCount];
            Material[] prevMaterials = r.sharedMaterials;
            Array.Copy(prevMaterials, materials, Math.Min(prevMaterials.Length, materials.Length));

            for (int si = 0; si < materialCount; ++si) {
                int mid = rec.materialIDs[si];
                Material material = FindMaterial(mid);
                if (material != null) {
                    if (materials[si] == material)
                        continue;
                    materials[si] = material;
                    changed = true;
                } else if (materials[si] == null) {
                    materials[si] = FindDefaultMaterial();
                    changed = true;
                }
            }

            if (!changed)
                return;
#if UNITY_EDITOR
            if (recordUndo)
                Undo.RecordObject(r, "Assign Material");
#endif
            r.sharedMaterials = materials;
        }

        internal bool EraseInstanceRecord<R>(
            Identifier identifier,
            Dictionary<string, R> records,
            Action<R> onErase)
        {
            var path = identifier.name;

            var ret = false;
            if (records.TryGetValue(path, out R rec)) {
                ret = records.Remove(path);

                onErase(rec);
            }

            return ret;
        }

        internal bool EraseInstanceInfoRecord(Identifier identifier)
        {
            return EraseInstanceRecord(identifier, m_clientInstances,
                delegate (InstanceInfoRecord record) {
                    DestroyImmediate(record.renderer);

                    record.DeleteInstanceObjects();

                    if (onDeleteInstanceInfo != null)
                        onDeleteInstancedEntity(identifier.name);
                });
        }

        internal bool EraseInstancedEntityRecord(Identifier identifier)
        {
            return EraseInstanceRecord(identifier, m_clientInstancedEntities,
                delegate (EntityRecord record)
                {
                    DestroyImmediate(record.go);
                    if (onDeleteInstancedEntity != null)
                        onDeleteInstancedEntity(identifier.name);
                });
        }

        internal bool EraseEntityRecord(Identifier identifier)
        {
            int id = identifier.id;
            string path = identifier.name;

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

        private bool ApplyMaterialList(MaterialList ml)
        {
            if (ml == null)
                return false;

            List<MaterialHolder> mats = ml.materials;
            if (mats != null && mats.Count != 0) {
                bool updated = false;
                int materialCount = Mathf.Min(m_materialList.Count, ml.materials.Count);
                for (int mi = 0; mi < materialCount; ++mi)
                {
                    MaterialHolder src = ml.materials[mi];
                    if (src.material == null)
                        continue;
                    MaterialHolder dst = m_materialList.Find(a => a.id == src.id);
                    if (dst == null)
                        continue;
                    dst.material = src.material;
                    updated = true;
                }
                if (updated)
                    ReassignMaterials(recordUndo: false);
            }

            if (ml.nodes == null)
                return true;

            foreach (MaterialList.Node node in ml.nodes) {
                Transform trans = FilmInternalUtilities.GameObjectUtility.FindByPath(m_rootObject, node.path);
                if (trans == null)
                    continue;

                Renderer r = trans.GetComponent<Renderer>();
                if (r != null)
                    r.sharedMaterials = node.materials;
            }

            return true;
        }

        private Material FindDefaultMaterial() {
            if (m_cachedDefaultMaterial != null)
                return m_cachedDefaultMaterial;

            GameObject primitive = GameObject.CreatePrimitive(PrimitiveType.Plane);
            primitive.SetActive(false);
            m_cachedDefaultMaterial = primitive.GetComponent<MeshRenderer>().sharedMaterial;
            DestroyImmediate(primitive);
            return m_cachedDefaultMaterial;
        }

        //----------------------------------------------------------------------------------------------------------------------    
#if UNITY_EDITOR
        private void GenerateLightmapUV(GameObject go)
        {
            if (go == null)
                return;

            SkinnedMeshRenderer smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null)
            {
                Mesh mesh = smr.sharedMesh;
                if (mesh != null)
                {
                    List<Vector2> uv = new List<Vector2>();
                    mesh.GetUVs(0, uv);
                    if (uv.Count == 0)
                    {
                        Unwrapping.GenerateSecondaryUVSet(mesh);
                        Debug.Log("generated uv " + mesh.name);
                    }
                }
            }
        }
        private void GenerateLightmapUV()
        {
            foreach (KeyValuePair<string, EntityRecord> kvp in m_clientObjects)
                GenerateLightmapUV(kvp.Value.go);
            foreach (KeyValuePair<int, EntityRecord> kvp in m_hostObjects)
            {
                if (kvp.Value.mesh != null)
                    GenerateLightmapUV(kvp.Value.go);
            }
        }

        public void ExportMaterials(bool overwrite = true, bool useExistingOnes = false)
        {
            MakeSureAssetDirectoryExists();

            // need to avoid filename collision
            Misc.UniqueNameGenerator nameGenerator = new Misc.UniqueNameGenerator();
            string basePath = m_assetsFolder;

            bool needSaveAssets = false;
            Func<Material, Material> doExport = (Material mat) => {
                if (mat == null || IsAsset(mat))
                    return mat;

                string dstPath = string.Format("{0}/{1}.mat", basePath, nameGenerator.Gen(mat.name));
                Material existing = AssetDatabase.LoadAssetAtPath<Material>(dstPath);
                if (overwrite || existing == null) {
                    mat = Misc.OverwriteOrCreateAsset(mat, dstPath);
                    MeshSyncPlayerConfig config = GetConfigV();
                    if (config.Logging)
                        Debug.Log("exported material " + dstPath);

                    needSaveAssets = true;
                } else if (useExistingOnes && existing != null)
                    mat = existing;
                return mat;
            };

            foreach (MaterialHolder m in m_materialList)
                m.material = doExport(m.material); // material maybe updated by SaveAsset()

            if (needSaveAssets)
                AssetDatabase.SaveAssets();
            ReassignMaterials(recordUndo: false);
        }

    bool IsSubfolder(string parentPath, string childPath)
    {
        var parentUri = new Uri(parentPath);
        var childUri = new DirectoryInfo(childPath).Parent;
        while (childUri != null)
        {
            if (new Uri(childUri.FullName) == parentUri)
            {
                return true;
            }
            childUri = childUri.Parent;
        }
        return false;
    }

    internal virtual void ClearInstancePrefabs() {
        MeshSyncLogger.VerboseLog("Clearing instance prefabs.");
 
        transform.DestroyChildrenImmediate();

        foreach (var prefabHolder in m_prefabDict.Values) {
            var path = AssetDatabase.GetAssetPath(prefabHolder.prefab);

            // Also delete its asset:
            var deps = AssetDatabase.GetDependencies(path);
            var assetName = Path.ChangeExtension(path, "asset");
            if (deps.Contains(assetName)) {
                AssetDatabase.DeleteAsset(assetName);
            }

            AssetDatabase.DeleteAsset(path);
        }

        m_prefabDict.Clear();
    }

    void Export(KeyValuePair<string, EntityRecord> kvp, bool overwrite, bool useExistingOnes, string basePath, MeshSyncPlayerConfig config, Misc.UniqueNameGenerator nameGenerator)
    {
        Mesh mesh = kvp.Value.mesh;

        if (mesh == null)
        {
            //if(kvp.Value.dataType == EntityType.Mesh)
            //{
            //    Debug.LogError($"Mesh was lost on {kvp.Key}");
            //}

            return;
        }

        // If this is an asset but in another folder, still save it, if it changed:
        if (IsAsset(mesh))
        {
            var currentPath = Path.GetFullPath(GetAssetsFolder());
            var existingPath = Path.GetFullPath(AssetDatabase.GetAssetPath(mesh));
            if (IsSubfolder(currentPath, existingPath))
            {
                return;
            }
        }

        string dstPath = string.Format("{0}/{1}.asset", basePath, nameGenerator.Gen(mesh.name));
        Mesh existing = AssetDatabase.LoadAssetAtPath<Mesh>(dstPath);
        if (overwrite || existing == null)
        {
            mesh = Misc.OverwriteOrCreateAsset(mesh, dstPath);
            kvp.Value.mesh = mesh; // mesh maybe updated by SaveAsset()

            // Ensure the saved mesh is also linked on the meshfilter:
            if (kvp.Value.meshFilter != null)
            {
                kvp.Value.meshFilter.sharedMesh = mesh;
            }
            if (kvp.Value.skinnedMeshRenderer != null)
            {
                kvp.Value.skinnedMeshRenderer.sharedMesh = mesh;
            }

            if (config.Logging)
                Debug.Log("exported mesh " + dstPath);
        }
        else if (useExistingOnes && existing != null)
            kvp.Value.mesh = existing;
    }

    public void ExportMeshes(bool overwrite = true, bool useExistingOnes = false)
    {
        MakeSureAssetDirectoryExists();

        // need to avoid filename collision
        Misc.UniqueNameGenerator nameGenerator = new Misc.UniqueNameGenerator();
        string basePath = GetAssetsFolder();

        // export client meshes
        MeshSyncPlayerConfig config = GetConfigV();

        foreach (KeyValuePair<string, EntityRecord> kvp in m_clientObjects)
        {
            Export(kvp, overwrite, useExistingOnes, basePath, config, nameGenerator);
        }

        foreach (KeyValuePair<string, EntityRecord> kvp in m_clientInstancedEntities)
        {
            Export(kvp, overwrite, useExistingOnes, basePath, config, nameGenerator);
        }

        // replace existing meshes
        int n = 0;
        foreach (KeyValuePair<int, EntityRecord> kvp in m_hostObjects)
        {
            EntityRecord rec = kvp.Value;
            if (rec.go == null || rec.mesh == null)
                continue;

            rec.origMesh.Clear(); // make editor can recognize mesh has modified by CopySerialized()
            EditorUtility.CopySerialized(rec.mesh, rec.origMesh);
            rec.mesh = null;

            MeshFilter mf = rec.go.GetComponent<MeshFilter>();
            if (mf != null)
                mf.sharedMesh = rec.origMesh;

            SkinnedMeshRenderer smr = rec.go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null)
                smr.sharedMesh = rec.origMesh;

            if (config.Logging)
                Debug.Log("updated mesh " + rec.origMesh.name);
            ++n;
        }
        if (n > 0)
            AssetDatabase.SaveAssets();
    }

    internal bool ExportMaterialList(string path)
    {
        if (string.IsNullOrEmpty(path))
            return false;

        path = path.Replace(Application.dataPath, "Assets/");
        if (!path.EndsWith(".asset"))
            path = path + ".asset";

        MaterialList ml = ScriptableObject.CreateInstance<MaterialList>();
        // material list
        ml.materials = m_materialList;

        // extract per-node materials
        Action<Transform> exportNode = (n) => {
            foreach (Renderer r in n.GetComponentsInChildren<Renderer>())
            {
                ml.nodes.Add(new MaterialList.Node
                {
                    path = BuildPath(r.transform),
                    materials = r.sharedMaterials,
                });
            }
        };
        if (m_rootObject != null)
            exportNode(m_rootObject);
        else
            foreach (GameObject root in SceneManager.GetActiveScene().GetRootGameObjects())
                exportNode(root.transform);

        AssetDatabase.CreateAsset(ml, path);
        return true;
    }

    internal bool ImportMaterialList(string path) {
        if (string.IsNullOrEmpty(path))
            return false;

        path = path.Replace(Application.dataPath, "Assets/");

        MaterialList ml = AssetDatabase.LoadAssetAtPath<MaterialList>(path);
        return ApplyMaterialList(ml);
    }

    //Returns true if changed
    private bool IsMaterialAssignedInSceneView() {
        bool changed = false;
        foreach (KeyValuePair<string, EntityRecord> kvp in m_clientObjects) {
            EntityRecord rec = kvp.Value;
            if (rec.go != null && rec.go.activeInHierarchy) {
                Renderer mr = rec.go.GetComponent<Renderer>();
                if (mr == null || rec.mesh == null)
                    continue;

                Material[] materials = mr.sharedMaterials;
                int n = Math.Min(materials.Length, rec.materialIDs.Length);
                for (int si = 0; si < n; ++si) {
                    int mid = rec.materialIDs[si];
                    MaterialHolder matHolder = m_materialList.Find(a => a.id == mid);
                    if (matHolder == null || materials[si] == matHolder.material) 
                        continue;
                    matHolder.material = materials[si];
                    changed       = true;
                    break;
                }
            }
            if (changed)
                break;
        }

        return changed;
    }

   
//---------------------------------------------------------------------------------------------------------------------    

    internal List<AnimationClip> GetAnimationClips()
    {
        List<AnimationClip> ret = new List<AnimationClip>();

        Action<GameObject> gatherClips = (go) => {
            AnimationClip[] clips = null;
            clips = AnimationUtility.GetAnimationClips(go);
            if (clips != null && clips.Length > 0)
                ret.AddRange(clips);
        };

        gatherClips(gameObject);
        // process children. root children should be enough.
        Transform trans = transform;
        int childCount = trans.childCount;
        for (int ci = 0; ci < childCount; ++ci)
            gatherClips(transform.GetChild(ci).gameObject);
        return ret;
    }

    private void OnSceneViewGUI(SceneView sceneView) {
        MeshSyncPlayerConfig config = GetConfigV();
        if (null == config) //may happen after deleting prefabs (SceneCache)
            return;

        if (!config.SyncMaterialList) 
            return;

        if (Event.current.type != EventType.DragExited || Event.current.button != 0) 
            return;
        
        bool changed = IsMaterialAssignedInSceneView();
        if (!changed) 
            return;
        
        // assume last undo group is "Assign Material" performed by mouse drag & drop.
        // collapse reassigning materials into it.
        int group = Undo.GetCurrentGroup() - 1;
        ReassignMaterials(true);
        Undo.CollapseUndoOperations(@group);
        Undo.FlushUndoRecordObjects();
        ForceRepaint();
        m_onMaterialChangedInSceneViewCB?.Invoke();
    }
    
    internal void AssignMaterial(MaterialHolder holder, Material mat) {
        Undo.RegisterCompleteObjectUndo(this, "Assign Material");
        holder.material = mat;
        ReassignMaterials(true);
        Undo.FlushUndoRecordObjects();
        ForceRepaint();
    }            
    
#endif //UNITY_EDITOR
#endregion

//----------------------------------------------------------------------------------------------------------------------        
#region Events
#if UNITY_EDITOR
    void Reset() {
        // force disable batching for export
        MethodInfo method = typeof(UnityEditor.PlayerSettings).GetMethod("SetBatchingForPlatform", 
            BindingFlags.NonPublic | BindingFlags.Static);
        if (method == null) 
            return;
        method.Invoke(null, new object[] { BuildTarget.StandaloneWindows, 0, 0 });
        method.Invoke(null, new object[] { BuildTarget.StandaloneWindows64, 0, 0 });
    }
#endif

    protected virtual void OnDestroy() {
        m_tmpI.Dispose();
        m_tmpV2.Dispose();
        m_tmpV3.Dispose();
        m_tmpV4.Dispose();
        m_tmpC.Dispose();
    }

//----------------------------------------------------------------------------------------------------------------------
    /// <summary>
    /// Monobehaviour's OnEnable(). Can be overridden
    /// </summary>
    private protected virtual void OnEnable() {
#if UNITY_EDITOR
        SceneView.duringSceneGui += OnSceneViewGUI;
#endif
    }

    /// <summary>
    /// Monobehaviour's OnDisable(). Can be overridden
    /// </summary>
    private protected virtual void OnDisable()
    {
#if UNITY_EDITOR
        SceneView.duringSceneGui -= OnSceneViewGUI;
#endif
    }

    private void Update() {
#if AT_USE_HDRP
        UpdatePathTracingState();
#endif
    }

#endregion //Events

        internal int getNumObservers => m_observers?.Count ?? 0;

        /// <summary>
        /// Send MeshSync sync event
        /// </summary>
        /// <param name="data">Asset type synced</param>
        private void SendEventData(MeshSyncAnalyticsData data) {

            foreach (var observer in this.m_observers) {
                observer.OnNext(data);
            }
        }

        /// <inheritdoc/>
        public IDisposable Subscribe(IObserver<MeshSyncAnalyticsData> observer) {

            this.m_observers.Add(observer);
            return null;
        }

        //----------------------------------------------------------------------------------------------------------------------

        // [TODO-sin: 2020-12-14] m_assetsFolder only makes sense for MeshSyncServer because we need to assign a folder that
        // will keep the synced resources as edits are performed on the DCC tool side.
        // For SceneCachePlayer, m_assetsFolder is needed only when loading the file, so it should be passed as a parameter

    [SerializeField] private string  m_assetsFolder = null; // Always starts with "Assets"
    [SerializeField] private Transform m_rootObject;
    
    [Obsolete][SerializeField] private bool m_usePhysicalCameraParams = true;  
    [SerializeField] private bool m_useCustomCameraMatrices = true;
            
    [SerializeField] private protected List<MaterialHolder> m_materialList = new List<MaterialHolder>();
    [SerializeField] private           List<TextureHolder>  m_textureList  = new List<TextureHolder>();
    [SerializeField] private           List<AudioHolder>    m_audioList    = new List<AudioHolder>();
    [SerializeField] private           PrefabDictionary     m_prefabDict   = new PrefabDictionary();

    [SerializeField] string[]       m_clientObjects_keys;
    [SerializeField] EntityRecord[] m_clientObjects_values;
    [SerializeField] int[]          m_hostObjects_keys;
    [SerializeField] EntityRecord[] m_hostObjects_values;
    [SerializeField] GameObject[]   m_objIDTable_keys;
    [SerializeField] int[]          m_objIDTable_values;
    [SerializeField] int            m_objIDSeed = 0;

#pragma warning disable 414
    [HideInInspector][SerializeField] private int m_baseMeshSyncVersion = (int) BaseMeshSyncVersion.NO_VERSIONING;
#pragma warning restore 414
    private const int CUR_BASE_MESHSYNC_VERSION = (int) BaseMeshSyncVersion.INITIAL_0_10_0;
    
#if UNITY_EDITOR
    [SerializeField] bool m_sortEntities          = true;
    [SerializeField] bool m_foldSyncSettings      = true;
    [SerializeField] bool m_foldImportSettings    = true;
    [SerializeField] bool m_foldMisc              = true;
    [SerializeField] bool m_foldMaterialList      = true;
    [SerializeField] bool m_foldAnimationTweak    = true;
    [SerializeField] bool m_foldExportAssets      = true;
#endif

    private bool m_saveAssetsInScene            = true;
    private bool m_markMeshesDynamic            = false;
    private bool m_needReassignMaterials        = false;
    private bool m_keyValuesSerializationEnabled = true;

    private List<IObserver<MeshSyncAnalyticsData>> m_observers = new List<IObserver<MeshSyncAnalyticsData>>();
    private Material m_cachedDefaultMaterial;

    private readonly           Dictionary<string, EntityRecord> m_clientObjects = new Dictionary<string, EntityRecord>();
    private protected readonly Dictionary<int, EntityRecord>    m_hostObjects   = new Dictionary<int, EntityRecord>();
    private readonly           Dictionary<GameObject, int>      m_objIDTable    = new Dictionary<GameObject, int>();

    [SerializeField] private protected InstanceInfoRecordDictionary m_clientInstances = new InstanceInfoRecordDictionary();
    [SerializeField] private EntityRecordDictionary m_clientInstancedEntities = new EntityRecordDictionary();

    private protected Action m_onMaterialChangedInSceneViewCB = null;
    
#if AT_USE_HDRP
    private bool m_pathTracingExists      = false;
    private bool m_needToResetPathTracing = false;
#endif
        
//----------------------------------------------------------------------------------------------------------------------

    PinnedList<int>     m_tmpI  = new PinnedList<int>();
    PinnedList<Vector2> m_tmpV2 = new PinnedList<Vector2>();
    PinnedList<Vector3> m_tmpV3 = new PinnedList<Vector3>();
    PinnedList<Vector4> m_tmpV4 = new PinnedList<Vector4>();
    PinnedList<Color>   m_tmpC  = new PinnedList<Color>();

#if AT_USE_SPLINES
    PinnedList<float3> m_tmpFloat3 = new PinnedList<float3>();
#endif

        //----------------------------------------------------------------------------------------------------------------------

        enum BaseMeshSyncVersion {
        NO_VERSIONING = 0,  //Didn't have versioning in earlier versions
        INITIAL_0_10_0 = 1, //initial for version 0.10.0-preview 
    
    }
    
    
}

} //end namespace