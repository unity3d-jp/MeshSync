using System;
using System.Collections.Generic;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
internal class InstanceInfoRecord {
    public GameObject               go;
    public List<MeshSyncInstanceRenderer> renderers = new List<MeshSyncInstanceRenderer>();
    public List<GameObject>         instanceObjects = new List<GameObject>();
    public List<string>             parentPaths = new List<string>();

    public void DeleteInstanceObjects(Transform parentFilter = null) {
        if (parentFilter == null)
        {
            foreach (GameObject obj in instanceObjects)
            {
                UnityEngine.Object.DestroyImmediate(obj);
            }

            instanceObjects.Clear();
        }
        else
        {
            for (var i = instanceObjects.Count - 1; i >= 0; i--)
            {
                var obj = instanceObjects[i];
                if (obj == null || parentFilter != obj.transform.parent)
                {
                    continue;
                }

                UnityEngine.Object.DestroyImmediate(obj);
                instanceObjects.RemoveAt(i);
            }
        }
    }
}
}