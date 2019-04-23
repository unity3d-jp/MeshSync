#if UNITY_EDITOR
using System.Text;
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
            var log = new StringBuilder();
            DumpTransform(log, go.GetComponent<Transform>());

            var smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null)
            {
                var sm = smr.sharedMesh;

                var bones = smr.bones;
                var bps = sm.bindposes;
                int numBones = bones.Length;
                for (int bi = 0; bi < numBones; ++bi)
                {
                    log.AppendFormat("  bindpose {0}:\n", bones[bi].name);
                    for (int i = 0; i < 4; ++i)
                    {
                        var v = bps[bi].GetColumn(i);
                        log.AppendFormat("    {0}, {1}, {2}, {3}\n", v.x, v.y, v.z, v.w);
                    }
                }

                log.Append("\n");
                for (int bi = 0; bi < numBones; ++bi)
                    DumpTransform(log, bones[bi]);
            }
            Debug.Log(log);
        }
    }

    public static string DumpTransform(StringBuilder log, Transform trans)
    {
        var t = trans.position;
        var rq = trans.rotation;
        var re = rq.eulerAngles;
        var s = trans.lossyScale;

        log.AppendFormat("{0}:\n", trans.name);
        log.AppendFormat("  global position: {0}, {1}, {2}\n", t.x, t.y, t.z);
        log.AppendFormat("  global rotation (quat): {0}, {1}, {2}, {3}\n", rq.x, rq.y, rq.z, rq.w);
        log.AppendFormat("  global rotation (euler): {0}, {1}, {2}\n", re.x, re.y, re.z);
        log.AppendFormat("  global scale: {0}, {1}, {2}\n", s.x, s.y, s.z);
        return log.ToString();
    }

    [MenuItem("Debug/Dump Mesh")]
    public static void DumpMesh()
    {
        SkinnedMeshRenderer smr = null;
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
                    smr = go.GetComponent<SkinnedMeshRenderer>();
                    if (smr != null)
                        mesh = smr.sharedMesh;
                }
            }
        }
        if (mesh == null)
            return;

        var log = new StringBuilder();
        log.AppendFormat("Mesh \"{0}\"\n", mesh.name);

        if (smr != null)
        {
            var bones = smr.bones;
            log.AppendFormat("bones ({0}):\n", bones.Length);
            foreach (var b in bones)
                log.AppendFormat("  {0}\n", b == null ? "" : b.name);
        }

        {
            var vertices = mesh.vertices;
            log.AppendFormat("vertices ({0}):\n", vertices.Length);
            foreach (var p in vertices)
                log.AppendFormat("  {0}, {1}, {2}\n", p.x, p.y, p.z);
        }
        {
            var normals = mesh.normals;
            log.AppendFormat("normals:\n");
            foreach (var p in normals)
                log.AppendFormat("  {0}, {1}, {2}\n", p.x, p.y, p.z);
        }
        {
            var tangents = mesh.tangents;
            log.AppendFormat("tangents:\n");
            foreach (var p in tangents)
                log.AppendFormat("  {0}, {1}, {2}, {3}\n", p.x, p.y, p.z, p.w);
        }
        {
            var uv = mesh.uv;
            log.AppendFormat("uv1:\n");
            foreach (var p in uv)
                log.AppendFormat("  {0}, {1}\n", p.x, p.y);
        }
        {
            var weights = mesh.boneWeights;
            log.AppendFormat("weights:\n");
            foreach (var p in weights)
                log.AppendFormat("  {0}:{1}, {2}:{3}, {4}:{5}, {6}:{7}\n",
                    p.boneIndex0, p.weight0,
                    p.boneIndex1, p.weight1,
                    p.boneIndex2, p.weight2,
                    p.boneIndex3, p.weight3 );
        }
        Debug.Log(log);
    }
}
#endif // UNITY_EDITOR
