#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;


public class DebugTransform
{
    [MenuItem("Debug/Dump Transform")]
    public static void DumpTransform()
    {
        var go = Selection.activeGameObject;
        if (go != null)
        {
            string str = DumpTransform(go.GetComponent<Transform>());

            var smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null)
            {
                var sm = smr.sharedMesh;

                var bones = smr.bones;
                var bps = sm.bindposes;
                int numBones = bones.Length;
                for (int bi = 0; bi < numBones; ++bi)
                {
                    str += string.Format("  bindpose {0}:\n", bones[bi].name);
                    for (int i = 0; i < 4; ++i)
                    {
                        var v = bps[bi].GetColumn(i);
                        str += string.Format("    {0}, {1}, {2}, {3}\n", v.x, v.y, v.z, v.w);
                    }
                }

                str += "\n";
                for (int bi = 0; bi < numBones; ++bi)
                {
                    str += DumpTransform(bones[bi]);
                    str += "\n";
                }
            }
            Debug.Log(str);
        }
    }

    public static string DumpTransform(Transform trans)
    {
        var t = trans.position;
        var rq = trans.rotation;
        var re = rq.eulerAngles;
        var s = trans.lossyScale;

        string str = trans.name + ":\n";
        str += string.Format("  global position: {0}, {1} {2}\n", t.x, t.y, t.z);
        str += string.Format("  global rotation (quat): {0}, {1}, {2}, {3}\n", rq.x, rq.y, rq.z, rq.w);
        str += string.Format("  global rotation (euler): {0}, {1}, {2}\n", re.x, re.y, re.z);
        str += string.Format("  global scale: {0}, {1}, {2}\n", s.x, s.y, s.z);
        return str;
    }

}
#endif // UNITY_EDITOR
