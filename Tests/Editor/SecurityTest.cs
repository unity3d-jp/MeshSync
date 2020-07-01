using System;
using System.Collections;
using NUnit.Framework;
using UnityEditor.SceneManagement;
using UnityEngine.TestTools;

namespace Unity.MeshSync.Editor.Tests {
internal class SecurityTest  {
    [UnityTest]
    public IEnumerator BlockServerRootTraversal()  {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);

        MeshSyncServer mss = MeshSyncServerEditor.CreateMeshSyncServer(true);
        Assert.IsTrue(mss.IsServerStarted());
        
        yield return null;

        Assert.IsFalse(CanAccessFileOutsideServerRoot());
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    
    bool CanAccessFileOutsideServerRoot() {

        try {

            //curl "
            System.Diagnostics.Process process = new System.Diagnostics.Process {
                StartInfo = {
                    FileName = "curl",
                    UseShellExecute       = false,
                    RedirectStandardError = true,
                    RedirectStandardOutput = true,
                    Arguments             = $"--path-as-is  \"http://127.0.0.1:8080/../../../Packages/manifest.json\""         
                },
                EnableRaisingEvents = true
            };            
            process.Start();
            process.WaitForExit();

            int exitCode = process.ExitCode;
            Assert.IsTrue( 0 == exitCode);
            
                       
            string stdOutput = process.StandardOutput.ReadToEnd();
            return !string.IsNullOrEmpty(stdOutput);

        } catch (Exception ) {
            Assert.Fail();
            return false;
        }

    }


}

} //end namespace
