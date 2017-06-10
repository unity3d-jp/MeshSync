using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ.HumbleNormalEditor
{

    public partial class NormalEditor : MonoBehaviour
    {
#if UNITY_EDITOR


        int GetMouseVertex(Event e, bool allowBackface = false)
        {
            //if (Tools.current != Tool.None)
            //{
            //    Debug.Log(Tools.current);
            //    return -1;
            //}

            Ray mouseRay = HandleUtility.GUIPointToWorldRay(e.mousePosition);
            float minDistance = float.MaxValue;
            int found = -1;
            Quaternion rotation = GetComponent<Transform>().rotation;
            for (int i = 0; i < m_points.Length; i++)
            {
                Vector3 dir = m_points[i] - mouseRay.origin;
                float sqrDistance = Vector3.Cross(dir, mouseRay.direction).sqrMagnitude;
                bool forwardFacing = Vector3.Dot(rotation * m_normals[i], Camera.current.transform.forward) <= 0;
                if ((forwardFacing || allowBackface) && sqrDistance < minDistance && sqrDistance < 0.05f * 0.05f)
                {
                    minDistance = sqrDistance;
                    found = i;
                }
            }
            return found;
        }

        public void ApplySet(Vector3 v)
        {
            v = GetComponent<Transform>().worldToLocalMatrix.MultiplyVector(v).normalized;

            for (int i = 0; i < m_selection.Length; ++i)
            {
                float s = m_selection[i];
                if (s > 0.0f)
                {
                    m_normals[i] = Vector3.Lerp(m_normals[i], v, s).normalized;
                }
            }
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyMove(Vector3 move)
        {
            move = GetComponent<Transform>().worldToLocalMatrix.MultiplyVector(move);

            for (int i = 0; i < m_selection.Length; ++i)
            {
                float s = m_selection[i];
                if (s > 0.0f)
                {
                    m_normals[i] = (m_normals[i] + move * s).normalized;
                }
            }
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyRotate(Quaternion rot)
        {
            {
                var mat = GetComponent<Transform>().worldToLocalMatrix;
                Vector3 axis = Vector3.zero;
                float angle = 0.0f;
                rot.ToAngleAxis(out angle, out axis);
                axis = mat.MultiplyVector(axis).normalized;
                rot = Quaternion.AngleAxis(angle, axis);
            }

            for (int i = 0; i < m_selection.Length; ++i)
            {
                float s = m_selection[i];
                if (s > 0.0f)
                {
                    m_normals[i] = Vector3.Lerp(m_normals[i], rot * m_normals[i], s).normalized;
                }
            }
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyRotatePivot(Quaternion amount, Vector3 pivotPos, Quaternion pivotRot)
        {
            var trans = GetComponent<Transform>().localToWorldMatrix;
            neRotatePivot(m_points, m_selection, m_points.Length, ref trans, amount, pivotPos, pivotRot, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyScale(Vector3 amount, Vector3 pivotPos, Quaternion pivotRot)
        {
            var trans = GetComponent<Transform>().localToWorldMatrix;
            neScale(m_points, m_selection, m_points.Length, ref trans, amount, pivotPos, pivotRot, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public bool ApplyAdditiveBrush(bool useSelection, Vector3 pos, float radius, float falloff, float strength, Vector3 amount)
        {
            amount = GetComponent<Transform>().worldToLocalMatrix.MultiplyVector(amount).normalized;

            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            var selection = useSelection && m_numSelected > 0 ? m_selection : null;
            if (neBrushAdd(m_points, selection, m_points.Length, ref trans, pos, radius, strength, falloff, amount, m_normals) > 0)
            {
                ApplyMirroring();
                UpdateNormals();
                return true;
            }
            return false;
        }

        public bool ApplyPinchBrush(bool useSelection, Vector3 pos, float radius, float falloff, float strength, Vector3 baseDir, float offset, float pow)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            var selection = useSelection && m_numSelected > 0 ? m_selection : null;
            if (neBrushPinch(m_points, selection, m_points.Length, ref trans, pos, radius, strength, falloff, baseDir, offset, pow, m_normals) > 0)
            {
                ApplyMirroring();
                UpdateNormals();
                return true;
            }
            return false;
        }

        public bool ApplyEqualizeBrush(bool useSelection, Vector3 pos, float radius, float falloff, float strength)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            var selection = useSelection && m_numSelected > 0 ? m_selection : null;
            if (neBrushEqualize(m_points, selection, m_points.Length, ref trans, pos, radius, strength, falloff, m_normals) > 0)
            {
                ApplyMirroring();
                UpdateNormals();
                return true;
            }
            return false;
        }

        public bool ApplyResetBrush(bool useSelection, Vector3 pos, float radius, float falloff, float strength)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            var selection = useSelection && m_numSelected > 0 ? m_selection : null;
            if (neBrushLerp(m_points, selection, m_points.Length, ref trans, pos, radius, strength, falloff, m_baseNormals, m_normals) > 0)
            {
                ApplyMirroring();
                UpdateNormals();
                return true;
            }
            return false;
        }

        public void ResetNormals(bool useSelection)
        {
            if (!useSelection)
            {
                Array.Copy(m_baseNormals, m_normals, m_normals.Length);
            }
            else
            {
                for (int i = 0; i < m_normals.Length; ++i)
                    m_normals[i] = Vector3.Lerp(m_normals[i], m_baseNormals[i], m_selection[i]).normalized;
            }
            UpdateNormals();
        }

        public void PushUndo()
        {
            Undo.RecordObject(this, "NormalEditor");
            m_history.count++;
            if (m_history.normals != null && m_history.normals.Length == m_normals.Length)
                Array.Copy(m_normals, m_history.normals, m_normals.Length);
            else
                m_history.normals = (Vector3[])m_normals.Clone();
            Undo.FlushUndoRecordObjects();
            //Debug.Log("PushUndo(): " + m_history.count);
        }

        public void OnUndoRedo()
        {
            //Debug.Log("OnUndoRedo(): " + m_history.count);
            if (m_history.normals != null && m_normals != null && m_history.normals.Length == m_normals.Length)
            {
                Array.Copy(m_history.normals, m_normals, m_normals.Length);
                UpdateNormals();
            }
        }

        public void UpdateNormals(bool upload = true)
        {
            if (m_cbNormals != null)
                m_cbNormals.SetData(m_normals);

            if (m_meshTarget != null)
            {
                m_meshTarget.normals = m_normals;
                if (upload)
                    m_meshTarget.UploadMeshData(false);
            }
        }

        public void UpdateSelection()
        {
            int prevSelected = m_numSelected;

            float st = 0.0f;
            m_numSelected = 0;
            m_selectionPos = Vector3.zero;
            m_selectionNormal = Vector3.zero;
            int numPoints = m_points.Length;

            for (int i = 0; i < numPoints; ++i)
            {
                float s = m_selection[i];
                if (s > 0.0f)
                {
                    m_selectionPos += m_points[i] * s;
                    m_selectionNormal += m_normals[i] * s;
                    ++m_numSelected;
                    st += s;
                }
            }

            if (m_numSelected > 0)
            {
                var trans = GetComponent<Transform>();
                var matrix = trans.localToWorldMatrix;

                m_selectionPos /= st;
                m_selectionPos = matrix.MultiplyPoint(m_selectionPos);
                m_selectionNormal = matrix.MultiplyVector(m_selectionNormal).normalized;
                m_selectionRot = Quaternion.LookRotation(m_selectionNormal);
                m_settings.pivotPos = m_selectionPos;
                m_settings.pivotRot = m_selectionRot;
            }

            if(prevSelected == 0 && m_numSelected == 0)
            {
                // no need to upload
            }
            else
            {
                m_cbSelection.SetData(m_selection);
            }
        }

        public void RecalculateTangents()
        {
            m_meshTarget.RecalculateTangents();
            m_tangents = m_meshTarget.tangents;
            if(m_cbTangents == null)
                m_cbTangents = new ComputeBuffer(m_tangents.Length, 16);
            m_cbTangents.SetData(m_tangents);
        }


        public bool Raycast(Event e, ref Vector3 pos, ref int ti)
        {
            Ray ray = HandleUtility.GUIPointToWorldRay(e.mousePosition);
            float d = 0.0f;
            if(Raycast(ray, ref ti, ref d))
            {
                pos = ray.origin + ray.direction * d;
                return true;
            }
            return false;
        }

        public bool Raycast(Ray ray, ref int ti, ref float distance)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            bool ret = neRaycast(ray.origin, ray.direction,
                m_points, m_triangles, m_triangles.Length / 3, ref ti, ref distance, ref trans) > 0;
            return ret;
        }

        public Vector3 PickNormal(Vector3 pos, int ti)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return neTriangleInterpolation(m_points, m_triangles, m_normals, ref trans, pos, ti);
        }

        public Vector3 PickBaseNormal(Vector3 pos, int ti)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return neTriangleInterpolation(m_points, m_triangles, m_baseNormals, ref trans, pos, ti);
        }


        public bool SelectAll()
        {
            for (int i = 0; i < m_selection.Length; ++i)
                m_selection[i] = 1.0f;
            return m_selection.Length > 0;
        }

        public bool ClearSelection()
        {
            System.Array.Clear(m_selection, 0, m_selection.Length);
            return m_selection.Length > 0;
        }

        public static Vector2 ScreenCoord11(Vector2 v)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            if (cam == null) { return v; }
            return new Vector2(
                v.x / cam.pixelWidth * 2.0f - 1.0f,
                (1.0f - v.y / cam.pixelHeight) * 2.0f - 1.0f);
        }

        public bool SelectRect(Vector2 r1, Vector2 r2, float strength)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            if (cam == null) { return false; }

            var campos = cam.GetComponent<Transform>().position;
            var trans = GetComponent<Transform>().localToWorldMatrix;
            var mvp = cam.projectionMatrix * cam.worldToCameraMatrix * trans;
            r1.x = r1.x / cam.pixelWidth * 2.0f - 1.0f;
            r2.x = r2.x / cam.pixelWidth * 2.0f - 1.0f;
            r1.y = (1.0f - r1.y / cam.pixelHeight) * 2.0f - 1.0f;
            r2.y = (1.0f - r2.y / cam.pixelHeight) * 2.0f - 1.0f;
            var rmin = new Vector2(Math.Min(r1.x, r2.x), Math.Min(r1.y, r2.y));
            var rmax = new Vector2(Math.Max(r1.x, r2.x), Math.Max(r1.y, r2.y));

            return neRectSelection(m_points, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength,
                ref mvp, ref trans, rmin, rmax, campos, m_settings.selectFrontSideOnly) > 0;
        }

        public bool SelectLasso(Vector2[] points, float strength)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            if (cam == null) { return false; }

            var campos = cam.GetComponent<Transform>().position;
            var trans = GetComponent<Transform>().localToWorldMatrix;
            var mvp = cam.projectionMatrix * cam.worldToCameraMatrix * trans;

            return neLassoSelection(m_points, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength,
                ref mvp, ref trans, points, points.Length, campos, m_settings.selectFrontSideOnly) > 0;
        }

        public bool SelectSoft(Vector3 pos, float radius, float falloff, float strength)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return neSoftSelection(m_points, m_points.Length, ref trans, pos, radius, strength, falloff, m_selection) > 0;
        }

        public static Vector3 GetMirrorPlane(MirrorMode mirrorMode)
        {
            switch (mirrorMode)
            {
                case MirrorMode.RightToLeft:    return Vector3.left;
                case MirrorMode.LeftToRight:    return Vector3.right;
                case MirrorMode.ForwardToBack:  return Vector3.back;
                case MirrorMode.BackToForward:  return Vector3.forward;
                case MirrorMode.UpToDown:       return Vector3.down;
                case MirrorMode.DownToUp:       return Vector3.up;
            }
            return Vector3.up;
        }

        MirrorMode m_prevMirrorMode;

        public bool ApplyMirroring()
        {
            if (m_settings.mirrorMode == MirrorMode.None) return false;

            bool needsSetup = false;
            if (m_mirrorRelation == null || m_mirrorRelation.Length != m_points.Length)
            {
                m_mirrorRelation = new int[m_points.Length];
                needsSetup = true;
            }
            else if(m_prevMirrorMode != m_settings.mirrorMode)
            {
                m_prevMirrorMode = m_settings.mirrorMode;
                needsSetup = true;
            }

            Vector3 planeNormal = GetMirrorPlane(m_settings.mirrorMode);
            if (needsSetup)
            {
                if (neBuildMirroringRelation(m_points, m_baseNormals, m_points.Length, planeNormal, 0.001f, m_mirrorRelation) == 0)
                {
                    Debug.LogWarning("NormalEditor: this mesh seems not symmetric");
                    m_mirrorRelation = null;
                    m_settings.mirrorMode = MirrorMode.None;
                    return false;
                }
            }
            neApplyMirroring(m_mirrorRelation, m_normals.Length, planeNormal, m_normals);
            return true;
        }


        static Rect FromToRect(Vector2 start, Vector2 end)
        {
            Rect r = new Rect(start.x, start.y, end.x - start.x, end.y - start.y);
            if (r.width < 0)
            {
                r.x += r.width;
                r.width = -r.width;
            }
            if (r.height < 0)
            {
                r.y += r.height;
                r.height = -r.height;
            }
            return r;
        }

        public bool BakeToTexture(int width, int height, string path)
        {
            if (path == null || path.Length == 0)
                return false;

            m_matBake.SetBuffer("_BaseNormals", m_cbBaseNormals);
            m_matBake.SetBuffer("_BaseTangents", m_cbBaseTangents);

            var rt = new RenderTexture(width, height, 0, RenderTextureFormat.ARGBHalf);
            var tex = new Texture2D(rt.width, rt.height, TextureFormat.RGBAHalf, false);
            rt.Create();

            m_cmdDraw.Clear();
            m_cmdDraw.SetRenderTarget(rt);
            for (int si = 0; si < m_meshTarget.subMeshCount; ++si)
                m_cmdDraw.DrawMesh(m_meshTarget, Matrix4x4.identity, m_matBake, si, 0);
            Graphics.ExecuteCommandBuffer(m_cmdDraw);

            RenderTexture.active = rt;
            tex.ReadPixels(new Rect(0, 0, tex.width, tex.height), 0, 0, false);
            tex.Apply();
            RenderTexture.active = null;

            if (path.EndsWith(".png"))
                System.IO.File.WriteAllBytes(path, tex.EncodeToPNG());
            else
                System.IO.File.WriteAllBytes(path, tex.EncodeToEXR());


            DestroyImmediate(tex);
            DestroyImmediate(rt);

            return true;
        }

        public bool BakeToVertexColor()
        {
            var color = new Color[m_normals.Length];
            for (int i = 0; i < m_normals.Length; ++i)
            {
                color[i].r = m_normals[i].x * 0.5f + 0.5f;
                color[i].g = m_normals[i].y * 0.5f + 0.5f;
                color[i].b = m_normals[i].z * 0.5f + 0.5f;
                color[i].a = 1.0f;
            }
            m_meshTarget.colors = color;
            return true;
        }

        public bool LoadTexture(Texture tex)
        {
            if (tex == null)
                return false;

            bool packed = false;
            {
                var path = AssetDatabase.GetAssetPath(tex);
                var importer = AssetImporter.GetAtPath(path) as TextureImporter;
                if (importer != null)
                    packed = importer.textureType == TextureImporterType.NormalMap;
            }

            var cbUV = new ComputeBuffer(m_normals.Length, 8);
            cbUV.SetData(m_meshTarget.uv);

            m_csBakeFromMap.SetInt("_Packed", packed ? 1 : 0);
            m_csBakeFromMap.SetTexture(0, "_NormalMap", tex);
            m_csBakeFromMap.SetBuffer(0, "_UV", cbUV);
            m_csBakeFromMap.SetBuffer(0, "_Normals", m_cbBaseNormals);
            m_csBakeFromMap.SetBuffer(0, "_Tangents", m_cbBaseTangents);
            m_csBakeFromMap.SetBuffer(0, "_Dst", m_cbNormals);
            m_csBakeFromMap.Dispatch(0, m_normals.Length, 1, 1);

            m_cbNormals.GetData(m_normals);
            cbUV.Dispose();

            UpdateNormals();
            PushUndo();

            return true;
        }

        public bool LoadVertexColor()
        {
            var color = m_meshTarget.colors;
            if (color.Length != m_normals.Length)
                return false;

            for (int i = 0; i < color.Length; ++i)
            {
                m_normals[i].x = color[i].r * 2.0f - 1.0f;
                m_normals[i].y = color[i].g * 2.0f - 1.0f;
                m_normals[i].z = color[i].b * 2.0f - 1.0f;
            }
            UpdateNormals();
            PushUndo();
            return true;
        }

        public void ApplyEqualize(float radius, float strength)
        {
            var selection = m_numSelected > 0 ? m_selection : null;
            var mat = GetComponent<Transform>().localToWorldMatrix;
            neEqualize(m_points, selection, m_points.Length, ref mat, radius, strength, m_normals);
            UpdateNormals();
            PushUndo();
        }

        public void ApplyProjection(GameObject go)
        {
            if (go == null) { return; }

            Mesh mesh = null;
            {
                var mf = go.GetComponent<MeshFilter>();
                if (mf != null)
                    mesh = mf.sharedMesh;
                else
                {
                    var smi = go.GetComponent<SkinnedMeshRenderer>();
                    if (smi != null)
                    {
                        mesh = new Mesh();
                        smi.BakeMesh(mesh);
                    }
                }
            }
            if (mesh == null)
            {
                Debug.LogWarning("ProjectNormals(): projector has no mesh!");
                return;
            }

            var ptrans = go.GetComponent<Transform>().localToWorldMatrix;
            ApplyProjection(mesh, ptrans);
        }

        public void ApplyProjection(Mesh projector, Matrix4x4 ptrans)
        {
            var mat = GetComponent<Transform>().localToWorldMatrix;
            var ppoints = projector.vertices;
            var prtiangles = projector.triangles;
            var pnormals = projector.normals;
            var selection = m_numSelected > 0 ? m_selection : null;
            if (pnormals.Length == 0)
            {
                Debug.LogWarning("ProjectNormals(): projector mesh has no normals!");
                return;
            }

            neProjectNormals(m_points, m_baseNormals, selection, m_points.Length, ref mat,
                ppoints, pnormals, prtiangles, prtiangles.Length / 3, ref ptrans, m_normals);
            UpdateNormals();
            PushUndo();
        }


        [DllImport("MeshSyncServer")] static extern int neRaycast(
            Vector3 pos, Vector3 dir, Vector3[] vertices, int[] indices, int num_triangles,
            ref int tindex, ref float distance, ref Matrix4x4 trans);

        [DllImport("MeshSyncServer")] static extern Vector3 neTriangleInterpolation(
            Vector3[] vertices, int[] indices, Vector3[] normals, ref Matrix4x4 trans,
            Vector3 pos, int ti);

        [DllImport("MeshSyncServer")] static extern int neSoftSelection(
            Vector3[] vertices, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, float[] seletion);

        [DllImport("MeshSyncServer")] static extern int neRectSelection(
            Vector3[] vertices, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength,
            ref Matrix4x4 mvp, ref Matrix4x4 trans, Vector2 rmin, Vector2 rmax, Vector3 campos, bool frontfaceOnly);

        [DllImport("MeshSyncServer")] static extern int neLassoSelection(
            Vector3[] vertices, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength,
            ref Matrix4x4 mvp, ref Matrix4x4 trans, Vector2[] points, int num_points, Vector3 campos, bool frontfaceOnly);

        [DllImport("MeshSyncServer")] static extern int neBrushAdd(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, Vector3 amount, Vector3[] normals);

        [DllImport("MeshSyncServer")] static extern int neBrushPinch(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, Vector3 baseNormal, float offset, float pow, Vector3[] normals);

        [DllImport("MeshSyncServer")] static extern int neBrushEqualize(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, Vector3[] normals);

        [DllImport("MeshSyncServer")] static extern int neBrushLerp(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, Vector3[] baseNormals, Vector3[] normals);

        [DllImport("MeshSyncServer")] static extern int neRotatePivot(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Quaternion amount, Vector3 pivotPos, Quaternion pivotRot, Vector3[] normals);

        [DllImport("MeshSyncServer")] static extern int neScale(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Vector3 amount, Vector3 pivotPos, Quaternion pivotRot, Vector3[] normals);

        [DllImport("MeshSyncServer")] static extern int neEqualize(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            float radius, float strength, Vector3[] normals);

        [DllImport("MeshSyncServer")] static extern int neBuildMirroringRelation(
            Vector3[] vertices, Vector3[] base_normals, int num_vertices, Vector3 plane_normal, float epsilon, int[] relation);

        [DllImport("MeshSyncServer")] static extern void neApplyMirroring(
            int[] relation, int num_vertices, Vector3 plane_normal, Vector3[] normals);

        [DllImport("MeshSyncServer")] static extern void neProjectNormals(
            Vector3[] vertices, Vector3[] normals, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Vector3[] pvertices, Vector3[] pnormals, int[] pindices, int num_triangles, ref Matrix4x4 ptrans,
            Vector3[] dst);
#endif
    }
}
