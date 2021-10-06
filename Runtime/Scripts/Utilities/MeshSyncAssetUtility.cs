namespace Unity.MeshSync {

//[TODO-sin: 2021-10-6] Move to FIU
internal static class MeshSyncAssetUtility {
    
    
    public static string NormalizeAssetPathInEditor(string path) {
        if (string.IsNullOrEmpty(path))
            return null;

        string slashedPath = path.Replace('\\', '/');        
        string projectRoot = GetApplicationRootPath();

        if (!slashedPath.StartsWith(projectRoot)) 
            return slashedPath;
        
        string normalizedPath = slashedPath.Substring(projectRoot.Length);
        if (normalizedPath.Length > 0) {
            normalizedPath = normalizedPath.Substring(1); //1 for additional '/'           
        }

        return normalizedPath;
    }


    static string GetApplicationRootPath() {
        if (null != m_appRootPath)
            return m_appRootPath;

        m_appRootPath = System.IO.Directory.GetCurrentDirectory().Replace('\\','/') + "/Assets";
        return m_appRootPath;
    }
    
    
    private static string m_appRootPath = null;
    

}

} //end namespace