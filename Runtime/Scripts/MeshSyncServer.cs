#if UNITY_STANDALONE
using System;
#endif
using Unity.Collections;
using UnityEngine;

#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync {
[ExecuteInEditMode]
internal class MeshSyncServer : MeshSyncPlayer {
    
    protected override void InitInternalV() {
        
    }
    
//----------------------------------------------------------------------------------------------------------------------        
    
#region Getter/Setter
    internal bool IsServerStarted()             { return m_serverStarted;}
    internal bool IsAutoStart()                 { return m_autoStartServer; }
    internal int  GetServerPort()               { return m_serverPort; }
    internal void SetServerPort(int port )      { m_serverPort = port; }
#endregion

//----------------------------------------------------------------------------------------------------------------------        
    internal void SetAutoStartServer(bool autoStart) {
        m_autoStartServer = autoStart; 

#if UNITY_STANDALONE        
        if (m_autoStartServer && !m_serverStarted) {
            StartServer();
        }
#endif
    }
    
//----------------------------------------------------------------------------------------------------------------------        
    
#if UNITY_EDITOR
    public bool foldServerSettings
    {
        get { return m_foldServerSettings; }
        set { m_foldServerSettings = value; }
    }
#endif

//----------------------------------------------------------------------------------------------------------------------        


    internal bool DoesServerAllowPublicAccess() {
        bool ret = false;
#if UNITY_STANDALONE            
        ret = m_server.IsPublicAccessAllowed();
#endif
        return ret;
    }
    
    internal void StartServer()
    {
#if UNITY_STANDALONE            
        StopServer();

#if UNITY_EDITOR 
        //Deploy HTTP assets to StreamingAssets
        DeployStreamingAssets.Deploy();
#endif
        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        
        
        m_serverSettings.port = (ushort)m_serverPort;
        m_serverSettings.zUpCorrectionMode = (ZUpCorrectionMode) m_config.ZUpCorrection;

        m_server = Server.Start(ref m_serverSettings);
        m_server.fileRootPath = GetServerDocRootPath();
        m_server.AllowPublicAccess(runtimeSettings.GetServerPublicAccess());
        
        m_handler = OnServerMessage;

#if UNITY_EDITOR
        EditorApplication.update += PollServerEvents;
#endif
        if (m_config.Logging)
            Debug.Log("[MeshSync] Server started (port: " + m_serverSettings.port + ")");

        m_serverStarted = true;
#else
        Debug.LogWarning("[MeshSync] Server functions are not supported in non-Standalone platform");
#endif //UNITY_STANDALONE
    }

//----------------------------------------------------------------------------------------------------------------------        

    internal void StopServer() {
#if UNITY_STANDALONE            
        if (!m_server) 
            return;
        
#if UNITY_EDITOR
        EditorApplication.update -= PollServerEvents;
#endif
        m_server.Stop();
        m_server = default(Server);

        if (m_config.Logging)
            Debug.Log("[MeshSync] Server stopped (port: " + m_serverSettings.port + ")");

        m_serverStarted = false;
#else
        Debug.LogWarning("[MeshSync] Server functions are not supported in non-Standalone platform");
#endif //UNITY_STANDALONE
    }

//----------------------------------------------------------------------------------------------------------------------

    protected override void OnBeforeSerializeMeshSyncPlayerV() {
        
    }

    protected override void OnAfterDeserializeMeshSyncPlayerV() {
        m_version = CUR_SERVER_VERSION;
    }   
    
//----------------------------------------------------------------------------------------------------------------------
    

#if UNITY_STANDALONE
    #region Impl
    
    void CheckParamsUpdated()  {

        if (m_server) {
            m_server.zUpCorrectionMode = (ZUpCorrectionMode) m_config.ZUpCorrection;
        }
    }
    #endregion

    #region MessageHandlers
    public void PollServerEvents() {
        if (m_requestRestartServer) {
            m_requestRestartServer = false;
            StartServer();
        }
        if (m_captureScreenshotInProgress) {
            m_captureScreenshotInProgress = false;
            m_server.screenshotPath = "screenshot.png";
        }

        if (m_server.numMessages > 0)
            m_server.ProcessMessages(m_handler);
    }

    void OnServerMessage(MessageType type, IntPtr data) {
        Try(() => {
            switch (type) {
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

    void OnRecvGet(GetMessage mes) {
        m_server.BeginServe();
        foreach (Renderer mr in FindObjectsOfType<Renderer>())
            ServeMesh(mr, mes);
        foreach (MaterialHolder mat in m_materialList)
            ServeMaterial(mat.material, mes);
        m_server.EndServe();

        if (m_config.Logging)
            Debug.Log("[MeshSync] served");
    }

    void OnRecvDelete(DeleteMessage mes) {
        int numEntities = mes.numEntities;
        for (int i = 0; i < numEntities; ++i)
            EraseEntityRecord(mes.GetEntity(i));

        int numMaterials = mes.numMaterials;
        for (int i = 0; i < numMaterials; ++i)
            EraseMaterialRecord(mes.GetMaterial(i).id);
    }

    void OnRecvFence(FenceMessage mes) {
        if (mes.type == FenceMessage.FenceType.SceneBegin) {
            BeforeUpdateScene();
        } else if (mes.type == FenceMessage.FenceType.SceneEnd) {
            AfterUpdateScene();
            m_server.NotifyPoll(PollMessage.PollType.SceneUpdate);
        }
    }

    void OnRecvText(TextMessage mes) {
        mes.Print();
    }

    void OnRecvSet(SetMessage mes) {
        UpdateScene(mes.scene);
    }

    void OnRecvScreenshot(IntPtr data) {
        ForceRepaint();

        ScreenCapture.CaptureScreenshot("screenshot.png");
        // actual capture will be done at end of frame. not done immediately.
        // just set flag now.
        m_captureScreenshotInProgress = true;
    }

    void OnRecvQuery(QueryMessage data) {
        switch (data.queryType) {
            case QueryMessage.QueryType.PluginVersion:
                data.AddResponseText(MeshSyncPlayer.GetPluginVersion());
                break;
            case QueryMessage.QueryType.ProtocolVersion:
                data.AddResponseText(MeshSyncPlayer.protocolVersion.ToString());
                break;
            case QueryMessage.QueryType.HostName:
                data.AddResponseText("Unity " + Application.unityVersion);
                break;
            case QueryMessage.QueryType.RootNodes:
                {
                    GameObject[] roots = UnityEngine.SceneManagement.SceneManager.GetActiveScene().GetRootGameObjects();
                    foreach (GameObject go in roots)
                        data.AddResponseText(BuildPath(go.transform));
                }
                break;
            case QueryMessage.QueryType.AllNodes: {
                Transform[] objTransforms = FindObjectsOfType<Transform>();
                foreach (Transform t in objTransforms)
                    data.AddResponseText(BuildPath(t));
                break;
            }
            default:
                break;
        }
        data.FinishRespond();
    }
    #endregion

    #region ServeScene
    bool ServeMesh(Renderer objRenderer, GetMessage mes) {
        bool ret = false;
        Mesh origMesh = null;

        MeshData dst = MeshData.Create();
        if (objRenderer.GetType() == typeof(MeshRenderer)){
            ret = CaptureMeshRenderer(ref dst, objRenderer as MeshRenderer, mes, ref origMesh);
        }
        else if (objRenderer.GetType() == typeof(SkinnedMeshRenderer)){
            ret = CaptureSkinnedMeshRenderer(ref dst, objRenderer as SkinnedMeshRenderer, mes, ref origMesh);
        }

        if (ret) {
            TransformData dstTrans = dst.transform;
            Transform rendererTransform = objRenderer.transform;
            dstTrans.hostID = GetObjectlID(objRenderer.gameObject);
            dstTrans.position = rendererTransform.localPosition;
            dstTrans.rotation = rendererTransform.localRotation;
            dstTrans.scale = rendererTransform.localScale;
            dst.local2world = rendererTransform.localToWorldMatrix;
            dst.world2local = rendererTransform.worldToLocalMatrix;

            EntityRecord rec;
            if (!m_hostObjects.TryGetValue(dstTrans.hostID, out rec)) {
                rec = new EntityRecord();
                m_hostObjects.Add(dstTrans.hostID, rec);
            }
            rec.go = objRenderer.gameObject;
            rec.origMesh = origMesh;

            dstTrans.path = BuildPath(rendererTransform);
            m_server.ServeMesh(dst);
        }
        return ret;
    }
    bool ServeTexture(Texture2D v, GetMessage mes) {
        TextureData data = TextureData.Create();
        data.name = v.name;
        // todo
        m_server.ServeTexture(data);
        return true;
    }
    bool ServeMaterial(Material mat, GetMessage mes) {
        MaterialData data = MaterialData.Create();
        data.name = mat.name;
        if (mat.HasProperty("_Color"))
            data.color = mat.GetColor("_Color");
        m_server.ServeMaterial(data);
        return true;
    }

    bool CaptureMeshRenderer(ref MeshData dst, MeshRenderer mr, GetMessage mes, ref Mesh mesh) {
        mesh = mr.GetComponent<MeshFilter>().sharedMesh;
        if (mesh == null) return false;
        if (!mesh.isReadable) {
            Debug.LogWarning("Mesh " + mr.name + " is not readable and be ignored");
            return false;
        }

        CaptureMesh(ref dst, mesh, null, mes.flags, mr.sharedMaterials);
        return true;
    }

    bool CaptureSkinnedMeshRenderer(ref MeshData dst, SkinnedMeshRenderer smr, GetMessage mes, ref Mesh mesh) {
        mesh = smr.sharedMesh;
        if (mesh == null) return false;

        if (!mes.bakeSkin && !mesh.isReadable) {
            Debug.LogWarning("Mesh " + smr.name + " is not readable and be ignored");
            return false;
        }

        Cloth cloth = smr.GetComponent<Cloth>();
        if (cloth != null && mes.bakeCloth) {
            CaptureMesh(ref dst, mesh, cloth, mes.flags, smr.sharedMaterials);
        }

        if (mes.bakeSkin) {
            Mesh tmp = new Mesh();
            smr.BakeMesh(tmp);
            CaptureMesh(ref dst, tmp, null, mes.flags, smr.sharedMaterials);
        } else {
            CaptureMesh(ref dst, mesh, null, mes.flags, smr.sharedMaterials);

            // bones
            if (mes.flags.getBones) {
                dst.SetBonePaths(this, smr.bones);
                dst.bindposes = mesh.bindposes;

                NativeArray<byte>        bonesPerVertex = mesh.GetBonesPerVertex();
                NativeArray<BoneWeight1> weights        = mesh.GetAllBoneWeights();
                dst.WriteBoneWeightsV(ref bonesPerVertex, ref weights);
            }

            // blendshapes
            if (mes.flags.getBlendShapes && mesh.blendShapeCount > 0) {
                Vector3[] v = new Vector3[mesh.vertexCount];
                Vector3[] n = new Vector3[mesh.vertexCount];
                Vector3[] t = new Vector3[mesh.vertexCount];
                for (int bi = 0; bi < mesh.blendShapeCount; ++bi) {
                    BlendShapeData bd = dst.AddBlendShape(mesh.GetBlendShapeName(bi));
                    bd.weight = smr.GetBlendShapeWeight(bi);
                    int frameCount = mesh.GetBlendShapeFrameCount(bi);
                    for (int fi = 0; fi < frameCount; ++fi) {
                        mesh.GetBlendShapeFrameVertices(bi, fi, v, n, t);
                        float w = mesh.GetBlendShapeFrameWeight(bi, fi);
                        bd.AddFrame(w, v, n, t);
                    }
                }
            }
        }
        return true;
    }

    void CaptureMesh(ref MeshData data, Mesh mesh, Cloth cloth, GetFlags flags, Material[] materials) {
        if (flags.getPoints)
            data.WritePoints(mesh.vertices);
        if (flags.getNormals)
            data.WriteNormals(mesh.normals);
        if (flags.getTangents)
            data.WriteTangents(mesh.tangents);
        
        //UV
        if (flags.GetUV(0)) { data.WriteUV(0, mesh.uv); }
        if (flags.GetUV(1)) { data.WriteUV(1, mesh.uv2); }
        if (flags.GetUV(2)) { data.WriteUV(2, mesh.uv3); }
        if (flags.GetUV(3)) { data.WriteUV(3, mesh.uv4); }
        if (flags.GetUV(4)) { data.WriteUV(4, mesh.uv5); }
        if (flags.GetUV(5)) { data.WriteUV(5, mesh.uv6); }
        if (flags.GetUV(6)) { data.WriteUV(6, mesh.uv7); }
        if (flags.GetUV(7)) { data.WriteUV(7, mesh.uv8); }
               
        if (flags.getColors)
            data.WriteColors(mesh.colors);
        if (flags.getIndices) {
            if (!flags.getMaterialIDs || materials == null || materials.Length == 0) {
                data.WriteIndices(mesh.triangles);
            } else {
                int n = mesh.subMeshCount;
                for (int i = 0; i < n; ++i) {
                    int[] indices = mesh.GetIndices(i);
                    int mid = i < materials.Length ? GetMaterialIndex(materials[i]) : 0;
                    data.WriteSubmeshTriangles(indices, mid);
                }
            }
        }

        // bones & blendshapes are handled by CaptureSkinnedMeshRenderer()
    }
    #endregion //ServeScene


    #region Events

#if UNITY_EDITOR
    void OnValidate() {
        CheckParamsUpdated();
    }

    void Reset() {
        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        m_config = MeshSyncRuntimeSettings.CreatePlayerConfig(MeshSyncPlayerType.SERVER);
        m_serverPort = runtimeSettings.GetDefaultServerPort();
        
    }
#endif


    protected override void OnEnable() {
        base.OnEnable();
        if (m_autoStartServer) {
            m_requestRestartServer = true;
        }
    }

    protected override void OnDisable() {
        base.OnDisable();
        StopServer();
    }

    void LateUpdate() {
        PollServerEvents();
    }
    #endregion

//----------------------------------------------------------------------------------------------------------------------


    ServerSettings m_serverSettings = ServerSettings.defaultValue;
    Server m_server;
    Server.MessageHandler m_handler;
    bool m_requestRestartServer = false;
    bool m_captureScreenshotInProgress = false;
    
#endif // UNITY_STANDALONE
    
    [SerializeField] private bool m_autoStartServer = false;
    [SerializeField]         int  m_serverPort      = MeshSyncConstants.DEFAULT_SERVER_PORT;
#if UNITY_EDITOR
    [SerializeField] bool m_foldServerSettings = true;
#endif

#pragma warning disable 414
    [HideInInspector][SerializeField] private int m_version = (int) ServerVersion.NO_VERSIONING;
#pragma warning restore 414
    private const int CUR_SERVER_VERSION = (int) ServerVersion.INITIAL_0_4_0;
    
    
    private bool m_serverStarted = false;

//----------------------------------------------------------------------------------------------------------------------    
    
    enum ServerVersion {
        NO_VERSIONING = 0, //Didn't have versioning in earlier versions        
        INITIAL_0_4_0 = 1, //initial for version 0.4.0-preview 
    
    }
    
}

} //end namespace
