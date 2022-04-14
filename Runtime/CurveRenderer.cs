using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync
{
    [ExecuteInEditMode]
    [RequireComponent(typeof(Curve))]
    internal class CurveRenderer : MonoBehaviour
    {
        private const float pointHandleSize = 0.05f;
        private const float pointPickSize = 0.05f;

        public Curve curve;

        Transform cachedTransform;

        private void OnEnable()
        {
            curve = GetComponent<Curve>();
            cachedTransform = transform;
        }

        private void OnDrawGizmos()
        {
            if (!Selection.Contains(gameObject))
            {
                Draw(false);
            }
        }

        public void Draw(bool drawHandles)
        {
            GUIStyle pointTextStyle = new GUIStyle();
            pointTextStyle.normal.textColor = Color.yellow;

            var width = drawHandles ? 5 : 1.5f;

            for (int splineIdx = 0; splineIdx < curve.splines.Length; splineIdx++)
            {
                Vector3? pos = null;
                Vector3 handle = Vector3.zero;

                var spline = curve.splines[splineIdx];

                for (int pointIdx = 0; pointIdx < spline.NumPoints; pointIdx++)
                {
                    var point = spline.GetPoint(pointIdx);

                    var transformedCo = cachedTransform.TransformPoint(point.co);

                    Vector3 activeHandle;
                    if (pointIdx == 0)
                    {
                        activeHandle = point.handle_right;
                    }
                    else
                    {
                        activeHandle = point.handle_left;
                    }

                    if (pos != null)
                    {
                        Handles.DrawBezier(
                            pos.Value,
                            transformedCo,
                            cachedTransform.TransformPoint(handle),
                            cachedTransform.TransformPoint(activeHandle),
                            new Color(1, 1, 1, 0.5f), null, width);
                    }

                    // Scale handle based on camera world position
                    float handleScale = HandleUtility.GetHandleSize(transformedCo);

                    if (drawHandles)
                    {
                        Handles.color = Color.gray;

                        if (curve.selectedPointIndex == pointIdx && curve.selectedSplineIndex == splineIdx)
                        {
                            Handles.color = Color.white;

                            const int handleLength = 1;
                            var posHandle_right = cachedTransform.TransformPoint(point.handle_right) * handleLength;
                            var posHandle_left = cachedTransform.TransformPoint(point.handle_left) * handleLength;

                            EditorGUI.BeginChangeCheck();

                            Handles.DrawLine(transformedCo, posHandle_right, 1);
                            posHandle_right = Handles.DoPositionHandle(posHandle_right, Quaternion.identity);

                            Handles.DrawLine(transformedCo, posHandle_left, 1);
                            posHandle_left = Handles.DoPositionHandle(posHandle_left, Quaternion.identity);

                            var newTransformedCo = Handles.DoPositionHandle(transformedCo, Quaternion.identity);

                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(curve, "Move Point");
                                EditorUtility.SetDirty(curve);

                                // Move handles with co:
                                var coMovement = newTransformedCo - transformedCo;
                                posHandle_left += coMovement;
                                posHandle_right += coMovement;

                                transformedCo = newTransformedCo;

                                point.co = cachedTransform.InverseTransformPoint(transformedCo);
                                point.handle_right = cachedTransform.InverseTransformPoint(posHandle_right);
                                point.handle_left = cachedTransform.InverseTransformPoint(posHandle_left);

                                spline.SetPoint(pointIdx, point);

                                curve.IsDirty = true;
                            }
                        }

                        if (Handles.Button(transformedCo, Quaternion.identity, handleScale * pointHandleSize, handleScale * pointPickSize, Handles.DotHandleCap))
                        {
                            curve.selectedPointIndex = pointIdx;
                            curve.selectedSplineIndex = splineIdx;
                        }

                        Handles.Label(transformedCo, pointIdx.ToString(), pointTextStyle);
                    }

                    pos = transformedCo;
                    handle = point.handle_right;
                }
            }
        }
    }
}
