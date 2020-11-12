namespace Unity.MeshSync.Editor  {

internal static class DiagnosticsUtility {

    internal static System.Diagnostics.Process StartProcess(string appPath, 
        string arguments = "", 
        bool useShellExecute = true, 
        bool redirectStandardError = false,
        bool redirectStandardOutput = false)
    {

        System.Diagnostics.Process process = new System.Diagnostics.Process {
            StartInfo = {
                FileName              = appPath,
                UseShellExecute       = useShellExecute,
                RedirectStandardError = redirectStandardError,
                RedirectStandardOutput = redirectStandardOutput,
                Arguments             = arguments,    
            },
            EnableRaisingEvents = true
        };            
        process.Start();
        return process;
    }

}

} //end namespace