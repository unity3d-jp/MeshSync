#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;


public static class DebugCommands
{
    [MenuItem("Debug/Open Log Directory")]
    public static void Open()
    {
        var path = System.Environment.GetEnvironmentVariable("LOCALAPPDATA") + "\\Unity\\Editor";
        System.Diagnostics.Process.Start(path);
    }


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
        str += string.Format("  global position: {0}, {1}, {2}\n", t.x, t.y, t.z);
        str += string.Format("  global rotation (quat): {0}, {1}, {2}, {3}\n", rq.x, rq.y, rq.z, rq.w);
        str += string.Format("  global rotation (euler): {0}, {1}, {2}\n", re.x, re.y, re.z);
        str += string.Format("  global scale: {0}, {1}, {2}\n", s.x, s.y, s.z);
        return str;
    }

    [MenuItem("Debug/Dump Mesh")]
    public static void DumpMesh()
    {
        var mesh = Selection.activeObject as Mesh;
        if (mesh == null)
        {
            var go = Selection.activeGameObject;
            if (go != null)
            {
                {
                    var mf = go.GetComponent<MeshFilter>();
                    if (mf != null)
                        mesh = mf.sharedMesh;
                }
                if (mesh == null)
                {
                    var smr = go.GetComponent<SkinnedMeshRenderer>();
                    if (smr != null)
                        mesh = smr.sharedMesh;
                }
            }
        }
        if (mesh == null)
            return;

        string log = "Mesh \"" + mesh.name + "\"\n";
        {
            var vertices = mesh.vertices;
            log += "vertices (" + vertices.Length + "):";
            foreach (var p in vertices)
                log += string.Format("  {0}, {1}, {2}\n", p.x, p.y, p.z);
        }
        {
            var normals = mesh.normals;
            log += "normals:";
            foreach (var p in normals)
                log += string.Format("  {0}, {1}, {2}\n", p.x, p.y, p.z);
        }
        {
            var tangents = mesh.tangents;
            log += "tangents:";
            foreach (var p in tangents)
                log += string.Format("  {0}, {1}, {2}, {3}\n", p.x, p.y, p.z, p.w);
        }
        {
            var uv = mesh.uv;
            log += "uv1:";
            foreach (var p in uv)
                log += string.Format("  {0}, {1}\n", p.x, p.y);
        }
        {
            var weights = mesh.boneWeights;
            log += "weights:";
            foreach (var p in weights)
                log += string.Format("  {0}:{1}, {2}:{3}, {4}:{5}, {6}:{7}\n",
                    p.boneIndex0, p.weight0,
                    p.boneIndex1, p.weight1,
                    p.boneIndex2, p.weight2,
                    p.boneIndex3, p.weight3 );
        }
        Debug.Log(log);
    }
}
#endif // UNITY_EDITOR
