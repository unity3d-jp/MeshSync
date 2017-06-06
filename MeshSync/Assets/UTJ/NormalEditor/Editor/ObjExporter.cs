// based on http://wiki.unity3d.com/index.php?title=ExportOBJ (thanks!)

using UnityEngine;
using UnityEditor;
using System.Collections;
using System.IO;
using System.Text;
 
public class ObjExporterScript
{
    private static int StartIndex = 0;

    public static void Start()
    {
        StartIndex = 0;
    }
    public static void End()
    {
        StartIndex = 0;
    }
 
 
    public static string MeshToString(Mesh mesh, Material[] mats, Transform t) 
    {
        if (!mesh)
            return "####Error####";

        var points = mesh.vertices;
        var normals = mesh.normals;
        var uv = mesh.uv;

        if (t != null)
        {
            Quaternion r = t.localRotation;
            for (int i = 0; i < points.Length; ++i)
                points[i] = t.TransformPoint(points[i]);
            for (int i = 0; i < normals.Length; ++i)
                normals[i] = r * normals[i];
        }

        StringBuilder sb = new StringBuilder();

        foreach(Vector3 v in points)
            sb.Append(string.Format("v {0} {1} {2}\n", v.x, v.y, -v.z));
        sb.Append("\n");
        foreach(Vector3 n in normals)
            sb.Append(string.Format("vn {0} {1} {2}\n", -n.x, -n.y, n.z));
        sb.Append("\n");
        foreach (Vector3 u in uv)
            sb.Append(string.Format("vt {0} {1}\n", u.x, u.y));

        for (int sm = 0; sm < mesh.subMeshCount; sm++)
        {
            sb.Append("\n");
            if (mats != null && sm < mats.Length)
            {
                sb.Append("usemtl ").Append(mats[sm].name).Append("\n");
                sb.Append("usemap ").Append(mats[sm].name).Append("\n");
            }

            int[] triangles = mesh.GetTriangles(sm);
            for (int i = 0; i < triangles.Length; i += 3)
            {
                sb.Append(string.Format("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}\n", 
                    triangles[i]+1+StartIndex, triangles[i+1]+1+StartIndex, triangles[i+2]+1+StartIndex));
            }
        }
 
        StartIndex += points.Length;
        return sb.ToString();
    }
}
 
public class ObjExporter
{
    public static void DoExport(GameObject go, bool makeSubmeshes, string path)
    {
        ObjExporterScript.Start();
 
        StringBuilder meshString = new StringBuilder();
        meshString.Append("#" + go.name + ".obj"
                            + "\n#" + System.DateTime.Now.ToLongDateString() 
                            + "\n#" + System.DateTime.Now.ToLongTimeString()
                            + "\n#-------" 
                            + "\n\n");
 
        Transform t = go.transform;
        Vector3 originalPosition = t.position;
        t.position = Vector3.zero;
 
        if (!makeSubmeshes)
            meshString.Append("g ").Append(t.name).Append("\n");
        meshString.Append(processTransform(t, makeSubmeshes));
 
        WriteToFile(meshString.ToString(), path);
 
        t.position = originalPosition;
 
        ObjExporterScript.End();
        Debug.Log("Exported Mesh: " + path);
    }
 
    static string processTransform(Transform t, bool makeSubmeshes)
    {
        StringBuilder meshString = new StringBuilder();
 
        meshString.Append("#" + t.name
                        + "\n#-------" 
                        + "\n");
 
        if (makeSubmeshes)
            meshString.Append("g ").Append(t.name).Append("\n");

        Mesh mesh = null;
        Material[] materials = null;
        {
            var mf = t.GetComponent<MeshFilter>();
            if (mf != null)
                mesh = mf.sharedMesh;
            else
            {
                var smi = t.GetComponent<SkinnedMeshRenderer>();
                if (smi != null)
                    mesh = smi.sharedMesh;
            }

            var renderer = t.GetComponent<Renderer>();
            if (renderer != null)
                materials = renderer.sharedMaterials;
        }

        if (mesh != null)
            meshString.Append(ObjExporterScript.MeshToString(mesh, materials, t));

        for (int i = 0; i < t.childCount; i++)
            meshString.Append(processTransform(t.GetChild(i), makeSubmeshes));

        return meshString.ToString();
    }
 
    static void WriteToFile(string s, string filename)
    {
        using (StreamWriter sw = new StreamWriter(filename))
            sw.Write(s);
    }
}