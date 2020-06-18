using UnityEngine;
using Unity.MeshSync;

namespace Unity.MeshSync.Editor.Tests {

    
[ExecuteInEditMode]
[RequireComponent(typeof(MeshSyncPlayer))]
public class SceneEventHandler : MonoBehaviour {
    string m_log;

    void OnSceneUpdateBegin() {
        m_log = "SceneUpdateBegin\n";
    }

    void OnUpdateAudio(AudioClip tex, AudioData data) {
        m_log += "Audio: " + tex.name + "\n";
    }

    void OnUpdateTexture(Texture tex, TextureData data) {
        m_log += "Texture: " + tex.name + "\n";
    }

    void OnUpdateMaterial(Material mat, MaterialData data) {
        m_log += "Material: " + mat.name + "\n";
    }

    void OnUpdateEntity(GameObject obj, TransformData data) {
        m_log += "GameObject: " + obj.name + "\n";

    }

    void OnUpdateAnimation(AnimationClip anim, AnimationClipData data) {
        m_log += "AnimationClip: " + anim.name + "\n";
    }

    void OnDeleteEntity(GameObject obj) {
        m_log += "Delete: " + obj.name + "\n";
    }

    void OnSceneUpdateEnd() {
        m_log += "SceneUpdateEnd\n";
        Debug.Log(m_log);
    }


    void OnEnable() {
        var mss = GetComponent<MeshSyncPlayer>();
        if (mss != null) {
            mss.onSceneUpdateBegin += OnSceneUpdateBegin;
            mss.onSceneUpdateEnd += OnSceneUpdateEnd;
            mss.onUpdateAudio += OnUpdateAudio;
            mss.onUpdateTexture += OnUpdateTexture;
            mss.onUpdateMaterial += OnUpdateMaterial;
            mss.onUpdateEntity += OnUpdateEntity;
            mss.onUpdateAnimation += OnUpdateAnimation;
            mss.onDeleteEntity += OnDeleteEntity;
        }
    }

    void OnDisable() {
        var mss = GetComponent<MeshSyncPlayer>();
        if (mss != null) {
            mss.onSceneUpdateBegin -= OnSceneUpdateBegin;
            mss.onSceneUpdateEnd -= OnSceneUpdateEnd;
            mss.onUpdateAudio -= OnUpdateAudio;
            mss.onUpdateTexture -= OnUpdateTexture;
            mss.onUpdateMaterial -= OnUpdateMaterial;
            mss.onUpdateEntity -= OnUpdateEntity;
            mss.onUpdateAnimation -= OnUpdateAnimation;
            mss.onDeleteEntity -= OnDeleteEntity;
        }
    }
}

} //end namespace

