namespace Unity.MeshSync.Editor  {

internal static class DiagnosticsUtility {

    internal static void StartProcess(string appPath, bool useShellExecute = true, bool redirectStandardError = false, 
        string arguments = "") {

        System.Diagnostics.Process process = new System.Diagnostics.Process {
            StartInfo = {
                FileName              = appPath,
                UseShellExecute       = useShellExecute,
                RedirectStandardError = redirectStandardError,
                Arguments             = arguments,    
            },
            EnableRaisingEvents = true
        };            
        process.Start();            
    }

}

} //end namespace