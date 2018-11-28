using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.SceneManagement;
#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.SceneManagement;
#endif

namespace UTJ.MeshSync
{
    [ExecuteInEditMode]
    public class SceneSwitcher : MonoBehaviour
    {
        [SerializeField] string[] m_sceneNames;
        [SerializeField] int m_sceneIndex = 0;
        int m_sceneIndexPrev = -1;
        [SerializeField][HideInInspector] Scene m_currentScene;

        public string[] sceneNames
        {
            get { return m_sceneNames; }
            set { m_sceneNames = value; }
        }

        void UnloadScene()
        {
            if (!m_currentScene.IsValid() || !m_currentScene.isLoaded)
                return;

#if UNITY_EDITOR
            if (Application.isPlaying)
                SceneManager.UnloadSceneAsync(m_currentScene);
            else
                EditorSceneManager.CloseScene(m_currentScene, true);
#else
            SceneManager.UnloadSceneAsync(m_currentScene);
#endif
        }

        void ChangeScene(int index)
        {
            if (m_sceneNames == null || m_sceneNames.Length == 0)
                return;

            UnloadScene();

            var path = m_sceneNames[Mathf.Clamp(m_sceneIndex, 0, m_sceneNames.Length - 1)];
#if UNITY_EDITOR
            if (Application.isPlaying)
            {
                SceneManager.LoadScene(path, LoadSceneMode.Additive);
                m_currentScene = SceneManager.GetSceneByPath(path);
            }
            else
                m_currentScene = EditorSceneManager.OpenScene(path, OpenSceneMode.Additive);
#else
            SceneManager.LoadScene(path);
            m_currentScene = SceneManager.GetSceneByPath(path);
#endif
            m_sceneIndexPrev = index;
        }

        void OnDisable()
        {
            UnloadScene();
            m_sceneIndexPrev = -1;
        }

        void OnEnable()
        {
            Update();
        }

        void Update()
        {
            if (m_sceneIndex != m_sceneIndexPrev &&
                m_sceneIndex >= 0 &&
                m_sceneIndex < m_sceneNames.Length)
            {
                ChangeScene(m_sceneIndex);
            }
        }
    }
}