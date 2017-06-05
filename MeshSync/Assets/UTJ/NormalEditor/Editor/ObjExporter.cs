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
 
 
    public static string MeshToString(MeshFilter mf, Transform t) 
    {	
        Vector3 s = t.localScale;
        Vector3 p = t.localPosition;
        Quaternion r = t.localRotation;
 
 
        int numVertices = 0;
        Mesh mesh = mf.sharedMesh;
        if (!mesh)
        {
            return "####Error####";
        }
        Material[] mats = mf.GetComponent<Renderer>().sharedMaterials;
 
        StringBuilder sb = new StringBuilder();
 
        foreach(Vector3 vv in mesh.vertices)
        {
            Vector3 v = t.TransformPoint(vv);
            numVertices++;
            sb.Append(string.Format("v {0} {1} {2}\n",v.x,v.y,-v.z));
        }
        sb.Append("\n");
        foreach(Vector3 nn in mesh.normals) 
        {
            Vector3 v = r * nn;
            sb.Append(string.Format("vn {0} {1} {2}\n",-v.x,-v.y,v.z));
        }
        sb.Append("\n");
        foreach (Vector3 v in mesh.uv)
        {
            sb.Append(string.Format("vt {0} {1}\n",v.x,v.y));
        }
        for (int material = 0; material < mesh.subMeshCount; material++)
        {
            sb.Append("\n");
            sb.Append("usemtl ").Append(mats[material].name).Append("\n");
            sb.Append("usemap ").Append(mats[material].name).Append("\n");
 
            int[] triangles = mesh.GetTriangles(material);
            for (int i=0;i<triangles.Length;i+=3) {
                sb.Append(string.Format("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}\n", 
                    triangles[i]+1+StartIndex, triangles[i+1]+1+StartIndex, triangles[i+2]+1+StartIndex));
            }
        }
 
        StartIndex += numVertices;
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
        {
            meshString.Append("g ").Append(t.name).Append("\n");
        }
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
        {
            meshString.Append("g ").Append(t.name).Append("\n");
        }
 
        MeshFilter mf = t.GetComponent<MeshFilter>();
        if (mf)
        {
            meshString.Append(ObjExporterScript.MeshToString(mf, t));
        }
 
        for(int i = 0; i < t.childCount; i++)
        {
            meshString.Append(processTransform(t.GetChild(i), makeSubmeshes));
        }
 
        return meshString.ToString();
    }
 
    static void WriteToFile(string s, string filename)
    {
        using (StreamWriter sw = new StreamWriter(filename)) 
        {
            sw.Write(s);
        }
    }
}