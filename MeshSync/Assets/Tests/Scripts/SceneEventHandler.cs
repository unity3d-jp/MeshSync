using System.Collections.Generic;
using UnityEngine;
using UTJ.MeshSync;


[ExecuteInEditMode]
public class SceneEventHandler : MonoBehaviour
{
    void OnSceneEvents(SceneEventType t, object arg)
    {
        Debug.Log(t);

        if(t == SceneEventType.UpdateInProgress)
        {
            var a = arg as SceneUpdateArgs;
            Debug.Log(a.gameObjects);
            //Debug.Log(a.textures);
            //Debug.Log(a.materials);
            //Debug.Log(a.animations);
        }
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

