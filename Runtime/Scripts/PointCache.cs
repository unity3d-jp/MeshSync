using UnityEngine;

namespace Unity.MeshSync {
[ExecuteInEditMode]
internal class PointCache : MonoBehaviour {
    [SerializeField] public Vector3[]    points;
    [SerializeField] public Quaternion[] rotations;
    [SerializeField] public Vector3[]    scales;
    [SerializeField] public Bounds       bounds;

    public void Clear() {
    }
}

} //end namespace