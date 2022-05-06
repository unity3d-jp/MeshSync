using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace Unity.MeshSync
{
    /*
    [Serializable]
    internal class Curve : MonoBehaviour
    {
        [HideInInspector]
        public int selectedSplineIndex = -1;

        [HideInInspector]
        public int selectedPointIndex = -1;

        public CurveSpline[] splines;
        public bool IsDirty { get; set; }
    }

    [Serializable]
    internal class CurveSpline
    {
        [SerializeField]
        Vector3[] cos;

        [SerializeField]
        Vector3[] handles_left;

        [SerializeField]
        Vector3[] handles_right;

        [SerializeField]
        public int NumPoints { get; private set; }

        public void Deserialize(CurvesData data, int index, PinnedList<Vector3> m_tmpV3)
        {
            NumPoints = data.GetNumSplinePoints(index);
            m_tmpV3.Resize(NumPoints);

            data.ReadSplineCos(index, m_tmpV3);
            m_tmpV3.CopyTo(ref cos);

            data.ReadSplineHandlesLeft(index, m_tmpV3);
            m_tmpV3.CopyTo(ref handles_left);

            data.ReadSplineHandlesRight(index, m_tmpV3);
            m_tmpV3.CopyTo(ref handles_right);
        }

        public CurveSplinePoint GetPoint(int index)
        {
            return new CurveSplinePoint() { co = cos[index], handle_left = handles_left[index], handle_right = handles_right[index] };
        }

        public void SetPoint(int index, CurveSplinePoint p)
        {
            cos[index] = p.co;
            handles_left[index] = p.handle_left;
            handles_right[index] = p.handle_right;
        }
    }

    public class CurveSplinePoint
    {
        public Vector3 co;
        public Vector3 handle_left;
        public Vector3 handle_right;
    }
    */
}
