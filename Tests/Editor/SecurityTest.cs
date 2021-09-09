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

        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateSettings();
        MeshSyncServer mss = MeshSyncMenu.CreateMeshSyncServer(true);
        Assert.IsTrue(mss.IsServerStarted());       
        yield return null;

        //Check the original public access
        bool origPublicAccess = projectSettings.GetServerPublicAccess();
        UnityEngine.Assertions.Assert.AreEqual(origPublicAccess, mss.DoesServerAllowPublicAccess());
        yield return null;
        
        //Change the public access and check
        projectSettings.SetServerPublicAccess(!origPublicAccess);
        mss.StopServer();
        mss.StartServer();
        Assert.IsTrue(mss.IsServerStarted());       
        yield return null;
        UnityEngine.Assertions.Assert.AreEqual(projectSettings.GetServerPublicAccess(), mss.DoesServerAllowPublicAccess());        
        
        //Change back
        projectSettings.SetServerPublicAccess(origPublicAccess);
        UnityEngine.Assertions.Assert.AreEqual(origPublicAccess, projectSettings.GetServerPublicAccess());
        
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    
    
    bool CanAccessFileOutsideServerRoot() {

        try {

            //curl "
            System.Diagnostics.Process process = DiagnosticsUtility.StartProcess(
                "curl",
                $"--path-as-is  \"http://127.0.0.1:8080/../../../Packages/manifest.json\"",

                /*UseShellExecute=*/ false,
                /*RedirectStandardError=*/ true,
                /*RedirectStandardOutput=*/ true
            );
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
