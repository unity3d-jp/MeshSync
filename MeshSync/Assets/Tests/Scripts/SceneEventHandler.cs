using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UTJ.MeshSync;


[ExecuteInEditMode]
public class SceneEventHandler : MonoBehaviour
{
    void Stringnize<T>(List<T> objs, string header, ref string dst) where T : UnityEngine.Object
    {
        if (objs.Count > 0)
        {
            dst += header;
            dst += " ";
            dst += string.Join(", ", objs.AsEnumerable().Select(o => o.name).ToArray());
            dst += "\n";
        }

    }

    void OnSceneEvents(SceneEventType t, object arg)
    {
        if (t == SceneEventType.Update)
        {
            var a = arg as SceneUpdateArgs;

            string log = "";
            Stringnize(a.textures, "Textures: ", ref log);
            Stringnize(a.materials, "Materials: ", ref log);
            Stringnize(a.gameObjects, "GameObjects: ", ref log);
            Stringnize(a.animations, "Animations: ", ref log);
            Debug.Log(log);
        }
        else
        {
            Debug.Log(t);
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

