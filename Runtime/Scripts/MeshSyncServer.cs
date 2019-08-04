using System;
using System.Linq;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
#if UNITY_2017_1_OR_NEWER
using UnityEngine.Animations;
#endif
#if UNITY_2019_1_OR_NEWER
using Unity.Collections;
#endif

#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.MeshSync
{
    [ExecuteInEditMode]
    public class MeshSyncServer : MeshSyncPlayer
    {
        #region Fields
        [SerializeField] int m_serverPort = ServerSettings.defaultPort;

        [HideInInspector] [SerializeField] int m_serverPortPrev = 0;

        ServerSettings m_serverSettings = ServerSettings.defaultValue;
        Server m_server;
        Server.MessageHandler m_handler;
        bool m_requestRestartServer = true;
        bool m_captureScreenshotInProgress = false;

        #endregion

        #region Properties
        public int serverPort
        {
            get { return m_serverPort; }
            set { m_serverPort = value; CheckParamsUpdated(); }
        }
        #endregion


        #region Impl
        void StartServer()
        {
            StopServer();

            m_serverSettings.port = (ushort)m_serverPort;
            m_serverSettings.zUpCorrectionMode = m_zUpCorrection;
            m_server = Server.Start(ref m_serverSettings);
            m_server.fileRootPath = httpFileRootPath;
            m_handler = OnServerMessage;
#if UNITY_EDITOR
            EditorApplication.update += PollServerEvents;
#if UNITY_2019_1_OR_NEWER
            SceneView.duringSceneGui += OnSceneViewGUI;
#else
            SceneView.onSceneGUIDelegate += OnSceneViewGUI;
#endif
#endif
            if (m_logging)
                Debug.Log("MeshSync: server started (port: " + m_serverSettings.port + ")");
        }

        void StopServer()
        {
            if (m_server)
            {
#if UNITY_EDITOR
                EditorApplication.update -= PollServerEvents;
#if UNITY_2019_1_OR_NEWER
                SceneView.duringSceneGui -= OnSceneViewGUI;
#else
                SceneView.onSceneGUIDelegate -= OnSceneViewGUI;
#endif
#endif
                m_server.Stop();
                m_server = default(Server);

                if (m_logging)
                    Debug.Log("MeshSync: server stopped (port: " + m_serverSettings.port + ")");
            }
        }

        void CheckParamsUpdated()
        {
            if (m_serverPort != m_serverPortPrev)
            {
                m_serverPortPrev = m_serverPort;
                m_requestRestartServer = true;
            }
            if (m_server)
            {
                m_server.zUpCorrectionMode = m_zUpCorrection;
            }
        }

        #endregion

        #region MessageHandlers
        public void PollServerEvents()
        {
            if (m_requestRestartServer)
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
                EraseEntityRecord(mes.GetEntity(i));

            int numMaterials = mes.numMaterials;
            for (int i = 0; i < numMaterials; ++i)
                EraseMaterialRecord(mes.GetMaterial(i).id);
        }

        void OnRecvFence(FenceMessage mes)
        {
            if (mes.type == FenceMessage.FenceType.SceneBegin)
            {
                BeforeUpdateScene();
            }
            else if (mes.type == FenceMessage.FenceType.SceneEnd)
            {
                AfterUpdateScene();
                m_server.NotifyPoll(PollMessage.PollType.SceneUpdate);
            }
        }

        void OnRecvText(TextMessage mes)
        {
            mes.Print();
        }

        void OnRecvSet(SetMessage mes)
        {
            UpdateScene(mes.scene);
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
                case QueryMessage.QueryType.PluginVersion:
                    data.AddResponseText(MeshSyncPlayer.pluginVersion.ToString());
                    break;
                case QueryMessage.QueryType.ProtocolVersion:
                    data.AddResponseText(MeshSyncPlayer.protocolVersion.ToString());
                    break;
                case QueryMessage.QueryType.HostName:
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
                var dstTrans = dst.transform;
                var trans = renderer.GetComponent<Transform>();
                dstTrans.hostID = GetObjectlID(renderer.gameObject);
                dstTrans.position = trans.localPosition;
                dstTrans.rotation = trans.localRotation;
                dstTrans.scale = trans.localScale;
                dst.local2world = trans.localToWorldMatrix;
                dst.world2local = trans.worldToLocalMatrix;

                EntityRecord rec;
                if (!m_hostObjects.TryGetValue(dstTrans.hostID, out rec))
                {
                    rec = new EntityRecord();
                    m_hostObjects.Add(dstTrans.hostID, rec);
                }
                rec.go = renderer.gameObject;
                rec.origMesh = origMesh;

                dstTrans.path = BuildPath(renderer.GetComponent<Transform>());
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

                // bones
                if (mes.flags.getBones)
                {
                    dst.SetBonePaths(this, smr.bones);
                    dst.bindposes = mesh.bindposes;

#if UNITY_2019_1_OR_NEWER
                    var bonesPerVertex = mesh.GetBonesPerVertex();
                    var weights = mesh.GetAllBoneWeights();
                    dst.WriteBoneWeightsV(ref bonesPerVertex, ref weights);
#else
                    dst.WriteBoneWeights4(mesh.boneWeights);
#endif
                }

                // blendshapes
                if (mes.flags.getBlendShapes && mesh.blendShapeCount > 0)
                {
                    var v = new Vector3[mesh.vertexCount];
                    var n = new Vector3[mesh.vertexCount];
                    var t = new Vector3[mesh.vertexCount];
                    for (int bi = 0; bi < mesh.blendShapeCount; ++bi)
                    {
                        var bd = dst.AddBlendShape(mesh.GetBlendShapeName(bi));
                        bd.weight = smr.GetBlendShapeWeight(bi);
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
            return true;
        }

        void CaptureMesh(ref MeshData data, Mesh mesh, Cloth cloth, GetFlags flags, Material[] materials)
        {
            if (flags.getPoints)
                data.WritePoints(mesh.vertices);
            if (flags.getNormals)
                data.WriteNormals(mesh.normals);
            if (flags.getTangents)
                data.WriteTangents(mesh.tangents);
            if (flags.getUV0)
                data.WriteUV0(mesh.uv);
            if (flags.getUV1)
                data.WriteUV1(mesh.uv2);
            if (flags.getColors)
                data.WriteColors(mesh.colors);
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

            // bones & blendshapes are handled by CaptureSkinnedMeshRenderer()
        }
        #endregion


        #region Events
        void OnValidate()
        {
            CheckParamsUpdated();
        }

        void OnEnable()
        {
            m_requestRestartServer = true;
        }

        void OnDisable()
        {
            StopServer();
        }

        void LateUpdate()
        {
            PollServerEvents();
        }
        #endregion
    }
}
