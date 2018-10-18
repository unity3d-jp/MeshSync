using System.Collections.Generic;
using UnityEngine;
using UTJ.MeshSync;


[ExecuteInEditMode]
public class SceneEventHandler : MonoBehaviour
{
    void OnSceneEvents(SceneEventType t, params object[] args)
    {
        Debug.Log(t);
    }

    void OnEnable()
    {
        var mss = GetComponent<MeshSyncServer>();
        if (mss != null)
            mss.sceneEvents += OnSceneEvents;
    }

    void OnDisable()
    {
        var mss = GetComponent<MeshSyncServer>();
        if (mss != null)
            mss.sceneEvents -= OnSceneEvents;
    }
}

