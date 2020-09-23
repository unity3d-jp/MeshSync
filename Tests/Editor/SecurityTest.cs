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

        MeshSyncServer mss = MeshSyncMenu.CreateMeshSyncServer(true);
        Assert.IsTrue(mss.IsServerStarted());
        
        yield return null;

        Assert.IsFalse(CanAccessFileOutsideServerRoot());
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    [UnityTest]
    public IEnumerator SetServerPublicAccess() {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);

        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        MeshSyncServer mss = MeshSyncMenu.CreateMeshSyncServer(true);
        Assert.IsTrue(mss.IsServerStarted());       
        yield return null;

        //Check the original public access
        bool origPublicAccess = runtimeSettings.GetServerPublicAccess();
        UnityEngine.Assertions.Assert.AreEqual(origPublicAccess, mss.DoesServerAllowPublicAccess());
        yield return null;
        
        //Change the public access and check
        runtimeSettings.SetServerPublicAccess(!origPublicAccess);
        mss.StopServer();
        mss.StartServer();
        Assert.IsTrue(mss.IsServerStarted());       
        yield return null;
        UnityEngine.Assertions.Assert.AreEqual(runtimeSettings.GetServerPublicAccess(), mss.DoesServerAllowPublicAccess());        
        
        //Change back
        runtimeSettings.SetServerPublicAccess(origPublicAccess);
        UnityEngine.Assertions.Assert.AreEqual(origPublicAccess, runtimeSettings.GetServerPublicAccess());
        
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
