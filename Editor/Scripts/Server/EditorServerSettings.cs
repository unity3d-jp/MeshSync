using UnityEditor;
using UnityEngine;

[FilePath("Assets/ProjectSettings/EditorServerSettings.yml", FilePathAttribute.Location.ProjectFolder)]
internal class EditorServerSettings : ScriptableSingleton<EditorServerSettings> {

    private const string DIRTY_KEY = "MESHSYNC_AUTO_SETUP_SETTINGS_DIRTY";
    
    [SerializeField] private ushort m_port = 8081;

    [SerializeField] private bool m_active = true;

    internal ushort Port {
        get {return m_port; }
        set {
            
            if (m_port == value)
                return;
            
            m_port = value;
            Save(true);

            if (m_active) {
                Dirty = true;
            }
        }
    }

    internal bool Active {
        get { return m_active; }
        set {
            if (m_active == value)
                return;
            
            m_active = value;
            Save(true);

            Dirty = true;
        }
    }
    
    internal bool Dirty {
        get { return SessionState.GetBool(DIRTY_KEY, m_active); }
        set {
            if (SessionState.GetBool(DIRTY_KEY, m_active) == value)
                return;

            SessionState.SetBool(DIRTY_KEY, value);
        }
    }
}
