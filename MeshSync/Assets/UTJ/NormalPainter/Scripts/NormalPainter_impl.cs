using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif


namespace UTJ.NormalPainter
{

    public partial class NormalPainter : MonoBehaviour
    {
#if UNITY_EDITOR


        public void ApplyAssign(Vector3 v, bool local = false)
        {
            var trans = local ? Matrix4x4.identity : GetComponent<Transform>().localToWorldMatrix;

            npAssign(m_selection, m_points.Length, ref trans, v, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyMove(Vector3 move, bool local = false)
        {
            var trans = local ? Matrix4x4.identity : GetComponent< Transform>().localToWorldMatrix;

            npMove(m_selection, m_points.Length, ref trans, move, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyRotate(Quaternion amount, Quaternion pivotRot, bool local = false)
        {
            var t = GetComponent<Transform>();
            var trans = local ? Matrix4x4.identity : t.localToWorldMatrix;
            if (local)
            {
                pivotRot = Quaternion.identity;
            }

            npRotate(m_points, m_selection, m_points.Length, ref trans, amount, pivotRot, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyRotatePivot(Quaternion amount, Vector3 pivotPos, Quaternion pivotRot, bool local = false)
        {
            var t = GetComponent<Transform>();
            var trans = local ? Matrix4x4.identity : t.localToWorldMatrix;
            if (local)
            {
                pivotPos -= t.position;
                pivotRot = Quaternion.identity;
            }

            npRotatePivot(m_points, m_selection, m_points.Length, ref trans, amount, pivotPos, pivotRot, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public void ApplyScale(Vector3 amount, Vector3 pivotPos, Quaternion pivotRot, bool local = false)
        {
            var t = GetComponent<Transform>();
            var trans = local ? Matrix4x4.identity : t.localToWorldMatrix;
            if(local)
            {
                pivotPos -= t.position;
                pivotRot = Quaternion.identity;
            }
            npScale(m_points, m_selection, m_points.Length, ref trans, amount, pivotPos, pivotRot, m_normals);
            ApplyMirroring();
            UpdateNormals();
        }

        public bool ApplyAdditiveBrush(bool useSelection, Vector3 pos, float radius, float falloff, float strength, Vector3 amount)
        {
            amount = GetComponent<Transform>().worldToLocalMatrix.MultiplyVector(amount).normalized;

            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            var selection = useSelection && m_numSelected > 0 ? m_selection : null;
            if (npBrushAdd(m_points, selection, m_points.Length, ref trans, pos, radius, strength, falloff, amount, m_normals) > 0)
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
            if (npBrushPinch(m_points, selection, m_points.Length, ref trans, pos, radius, strength, falloff, baseDir, offset, pow, m_normals) > 0)
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
            if (npBrushEqualize(m_points, selection, m_points.Length, ref trans, pos, radius, strength, falloff, m_normals) > 0)
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
            if (npBrushLerp(m_points, selection, m_points.Length, ref trans, pos, radius, strength, falloff, m_baseNormals, m_normals) > 0)
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

            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            m_numSelected = npUpdateSelection(m_points, m_normals, m_selection, m_points.Length, ref trans,
                ref m_selectionPos, ref m_selectionNormal);

            m_selectionRot = Quaternion.identity;
            if (m_numSelected > 0)
            {
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
            bool ret = npRaycast(ray.origin, ray.direction,
                m_points, m_triangles, m_triangles.Length / 3, ref ti, ref distance, ref trans) > 0;
            return ret;
        }

        public Vector3 PickNormal(Vector3 pos, int ti)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return npTriangleInterpolation(m_points, m_triangles, m_normals, ref trans, pos, ti);
        }

        public Vector3 PickBaseNormal(Vector3 pos, int ti)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return npTriangleInterpolation(m_points, m_triangles, m_baseNormals, ref trans, pos, ti);
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
            var pixelRect = cam.pixelRect;
            var rect = cam.rect;
            return new Vector2(
                    v.x / pixelRect.width * rect.width * 2.0f - 1.0f,
                    (v.y / pixelRect.height * rect.height * 2.0f - 1.0f) * -1.0f);
        }

        public bool SelectSingle(Event e, float strength, bool frontFaceOnly)
        {
            var center = e.mousePosition;
            var size = new Vector2(15.0f, 15.0f);
            var r1 = center - size;
            var r2 = center + size;
            return SelectSingle(r1, r2, strength, frontFaceOnly);
        }
        public bool SelectSingle(Vector2 r1, Vector2 r2, float strength, bool frontFaceOnly)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            if (cam == null) { return false; }

            var campos = cam.GetComponent<Transform>().position;
            var trans = GetComponent<Transform>().localToWorldMatrix;
            var mvp = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false) * cam.worldToCameraMatrix * trans;
            r1 = ScreenCoord11(r1);
            r2 = ScreenCoord11(r2);
            var rmin = new Vector2(Math.Min(r1.x, r2.x), Math.Min(r1.y, r2.y));
            var rmax = new Vector2(Math.Max(r1.x, r2.x), Math.Max(r1.y, r2.y));

            return npSelectSingle(m_points, m_baseNormals, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength,
                ref mvp, ref trans, rmin, rmax, campos, frontFaceOnly) > 0;
        }

        public bool SelectRect(Vector2 r1, Vector2 r2, float strength, bool frontFaceOnly)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            if (cam == null) { return false; }

            var campos = cam.GetComponent<Transform>().position;
            var trans = GetComponent<Transform>().localToWorldMatrix;
            var mvp = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false) * cam.worldToCameraMatrix * trans;
            r1 = ScreenCoord11(r1);
            r2 = ScreenCoord11(r2);
            var rmin = new Vector2(Math.Min(r1.x, r2.x), Math.Min(r1.y, r2.y));
            var rmax = new Vector2(Math.Max(r1.x, r2.x), Math.Max(r1.y, r2.y));

            return npSelectRect(m_points, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength,
                ref mvp, ref trans, rmin, rmax, campos, frontFaceOnly) > 0;
        }

        public bool SelectLasso(Vector2[] points, float strength, bool frontFaceOnly)
        {
            var cam = SceneView.lastActiveSceneView.camera;
            if (cam == null) { return false; }

            var campos = cam.GetComponent<Transform>().position;
            var trans = GetComponent<Transform>().localToWorldMatrix;
            var mvp = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false) * cam.worldToCameraMatrix * trans;

            return npSelectLasso(m_points, m_triangles, m_points.Length, m_triangles.Length / 3, m_selection, strength,
                ref mvp, ref trans, points, points.Length, campos, frontFaceOnly) > 0;
        }

        public bool SelectSoft(Vector3 pos, float radius, float falloff, float strength)
        {
            Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
            return npSelectBrush(m_points, m_points.Length, ref trans, pos, radius, strength, falloff, m_selection) > 0;
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
                if (npBuildMirroringRelation(m_points, m_baseNormals, m_points.Length, planeNormal, 0.001f, m_mirrorRelation) == 0)
                {
                    Debug.LogWarning("NormalEditor: this mesh seems not symmetric");
                    m_mirrorRelation = null;
                    m_settings.mirrorMode = MirrorMode.None;
                    return false;
                }
            }
            npApplyMirroring(m_mirrorRelation, m_normals.Length, planeNormal, m_normals);
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
            npEqualize(m_points, selection, m_points.Length, ref mat, radius, strength, m_normals);
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

            npProjectNormals(m_points, m_baseNormals, selection, m_points.Length, ref mat,
                ppoints, pnormals, prtiangles, prtiangles.Length / 3, ref ptrans, m_normals);
            UpdateNormals();
            PushUndo();
        }


        [DllImport("NormalPainter")] static extern int npRaycast(
            Vector3 pos, Vector3 dir, Vector3[] vertices, int[] indices, int num_triangles,
            ref int tindex, ref float distance, ref Matrix4x4 trans);

        [DllImport("NormalPainter")] static extern Vector3 npTriangleInterpolation(
            Vector3[] vertices, int[] indices, Vector3[] normals, ref Matrix4x4 trans,
            Vector3 pos, int ti);

        [DllImport("NormalPainter")] static extern int npSelectSingle(
            Vector3[] vertices, Vector3[] normals, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength,
            ref Matrix4x4 mvp, ref Matrix4x4 trans, Vector2 rmin, Vector2 rmax, Vector3 campos, bool frontfaceOnly);

        [DllImport("NormalPainter")] static extern int npSelectRect(
            Vector3[] vertices, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength,
            ref Matrix4x4 mvp, ref Matrix4x4 trans, Vector2 rmin, Vector2 rmax, Vector3 campos, bool frontfaceOnly);

        [DllImport("NormalPainter")] static extern int npSelectLasso(
            Vector3[] vertices, int[] indices, int num_vertices, int num_triangles, float[] seletion, float strength,
            ref Matrix4x4 mvp, ref Matrix4x4 trans, Vector2[] points, int num_points, Vector3 campos, bool frontfaceOnly);
        
        [DllImport("NormalPainter")] static extern int npSelectBrush(
            Vector3[] vertices, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, float[] seletion);

        [DllImport("NormalPainter")] static extern int npUpdateSelection(
            Vector3[] vertices, Vector3[] normals, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            ref Vector3 selection_pos, ref Vector3 selection_normal);


        [DllImport("NormalPainter")] static extern int npBrushAdd(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, Vector3 amount, Vector3[] normals);

        [DllImport("NormalPainter")] static extern int npBrushPinch(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, Vector3 baseNormal, float offset, float pow, Vector3[] normals);

        [DllImport("NormalPainter")] static extern int npBrushEqualize(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, Vector3[] normals);

        [DllImport("NormalPainter")] static extern int npBrushLerp(
            Vector3[] vertices, float[] seletion, int num_vertices, ref Matrix4x4 trans,
            Vector3 pos, float radius, float strength, float falloff, Vector3[] baseNormals, Vector3[] normals);

        [DllImport("NormalPainter")] static extern int npAssign(
            float[] selection, int num_vertices, ref Matrix4x4 trans, Vector3 amount, Vector3[] normals);
        
        [DllImport("NormalPainter")] static extern int npMove(
            float[] selection, int num_vertices, ref Matrix4x4 trans, Vector3 amount, Vector3[] normals);
        
        [DllImport("NormalPainter")] static extern int npRotate(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Quaternion amount, Quaternion pivotRot, Vector3[] normals);

        [DllImport("NormalPainter")] static extern int npRotatePivot(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Quaternion amount, Vector3 pivotPos, Quaternion pivotRot, Vector3[] normals);

        [DllImport("NormalPainter")] static extern int npScale(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Vector3 amount, Vector3 pivotPos, Quaternion pivotRot, Vector3[] normals);

        [DllImport("NormalPainter")] static extern int npEqualize(
            Vector3[] vertices, float[] selection, int num_vertices, ref Matrix4x4 trans,
            float radius, float strength, Vector3[] normals);

        [DllImport("NormalPainter")] static extern int npBuildMirroringRelation(
            Vector3[] vertices, Vector3[] base_normals, int num_vertices, Vector3 plane_normal, float epsilon, int[] relation);

        [DllImport("NormalPainter")] static extern void npApplyMirroring(
            int[] relation, int num_vertices, Vector3 plane_normal, Vector3[] normals);

        [DllImport("NormalPainter")] static extern void npProjectNormals(
            Vector3[] vertices, Vector3[] normals, float[] selection, int num_vertices, ref Matrix4x4 trans,
            Vector3[] pvertices, Vector3[] pnormals, int[] pindices, int num_triangles, ref Matrix4x4 ptrans,
            Vector3[] dst);
#endif
    }
}
