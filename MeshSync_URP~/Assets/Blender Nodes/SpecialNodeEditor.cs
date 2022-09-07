using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

[ExecuteAlways]
public class SpecialNodeEditor : MonoBehaviour
{
    public Vector3 MostFirstPoint;
    public Vector3 MostLastPoint;
    public Vector2 RandomValue;
    Material sharedMaterial;

    void Start()
    {
        RandomValue = new Vector2(Random.Range(0, 300), Random.Range(0, 300));
        sharedMaterial = GetComponent<MeshRenderer>().sharedMaterial;
        Mesh mesh = GetComponent<MeshFilter>().sharedMesh;
        Vector3[] vertices = mesh.vertices;
        /*for (var i = 0; i < vertices.Length; i++)
        {
            Vector3 direction = transform.TransformPoint(vertices[i]);
            Debug.Log(vertices[i]);
        }*/
        for (int i = 0; i < vertices.Length; i++)
        {
            if (MostFirstPoint.x > vertices[i].x)
                MostFirstPoint.x = vertices[i].x;
            if (MostFirstPoint.y > vertices[i].y)
                MostFirstPoint.y = vertices[i].y;
            if (MostFirstPoint.z > vertices[i].z)
                MostFirstPoint.z = vertices[i].z;

            if (MostLastPoint.x < vertices[i].x)
                MostLastPoint.x = vertices[i].x;
            if (MostLastPoint.y < vertices[i].y)
                MostLastPoint.y = vertices[i].y;
            if (MostLastPoint.z < vertices[i].z)
                MostLastPoint.z = vertices[i].z;
        }

        GetComponent<MeshRenderer>().material.SetVector("_FirstPoint", MostFirstPoint);
        GetComponent<MeshRenderer>().material.SetVector("_LastPoint", MostLastPoint);
        GetComponent<MeshRenderer>().material.SetVector("_RandomVector", RandomValue);
    }

    private void OnDestroy()
    {
        GetComponent<MeshRenderer>().material = sharedMaterial;
    }
}

[CustomEditor(typeof(SpecialNodeEditor))]
public class TestOnInspector : Editor
{
    SerializedProperty FirstPoint;
    SerializedProperty LastPoint;
    SerializedProperty RandomValue;
    private void OnEnable()
    {
        FirstPoint = serializedObject.FindProperty("MostFirstPoint");
        LastPoint = serializedObject.FindProperty("MostLastPoint");
        RandomValue = serializedObject.FindProperty("RandomValue");
    }
    public override void OnInspectorGUI()
    {
        EditorGUILayout.HelpBox("If this script throws error about sharedMaterial it is OK!", MessageType.Info);
        EditorGUILayout.PropertyField(FirstPoint);
        EditorGUILayout.PropertyField(LastPoint);
        EditorGUILayout.PropertyField(RandomValue);
    }
}