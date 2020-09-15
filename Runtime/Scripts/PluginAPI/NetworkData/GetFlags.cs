namespace Unity.MeshSync
{

internal struct GetFlags {
    private BitFlags m_flags;
    public bool     getTransform   { get { return m_flags[0]; } }
    public bool     getPoints      { get { return m_flags[1]; } }
    public bool     getNormals     { get { return m_flags[2]; } }
    public bool     getTangents    { get { return m_flags[3]; } }
    public bool     getColors      { get { return m_flags[6]; } }
    public bool     getIndices     { get { return m_flags[7]; } }
    public bool     getMaterialIDs { get { return m_flags[8]; } }
    public bool     getBones       { get { return m_flags[9]; } }
    public bool     getBlendShapes { get { return m_flags[10]; } }

    const  int  UV_START_BIT_POS = 24;
    public bool GetUV(int index) { return m_flags[UV_START_BIT_POS + index]; } 
        
}

} //end namespace
