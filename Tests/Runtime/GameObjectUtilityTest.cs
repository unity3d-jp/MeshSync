using NUnit.Framework;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace Unity.MeshSync.Tests {
internal class GameObjectUtilityTest {

    [Test]
    public void TestSetup() {
        Transform created_d  = FindOrCreateGameObject(null, "a/b/c0/d");
        Transform created_c0 = FindOrCreateGameObject(null, "a/b/c0");
        Transform created_c1 = FindOrCreateGameObject(null, "a/b/c1");
        Transform created_b  = FindOrCreateGameObject(null, "a/b");
        Transform created_a  = FindOrCreateGameObject(null, "a");

        Transform a  = VerifySingleRootGameObjectExists("a");
        Transform b  = VerifySingleChildExists(a,"b");
        Transform c0 = VerifySingleChildExists(b,"c0");
        Transform c1 = VerifySingleChildExists(b,"c1");
        Transform d  = VerifySingleChildExists(c0,"d");
        
        Assert.AreEqual(a,  created_a);
        Assert.AreEqual(b,  created_b);
        Assert.AreEqual(c0, created_c0);
        Assert.AreEqual(c1, created_c1);
        Assert.AreEqual(d,  created_d);
    }

//----------------------------------------------------------------------------------------------------------------------        
    
    [Test]
    public void FindGameObjectsByPath() {

        Transform created_d  = FindOrCreateGameObject(null, "a/b/c0/d");
        Transform created_c0 = FindOrCreateGameObject(null, "a/b/c0");
        Transform created_c1 = FindOrCreateGameObject(null, "a/b/c1");
        Transform created_b  = FindOrCreateGameObject(null, "a/b");
        Transform created_a  = FindOrCreateGameObject(null, "a");
        

        Transform a  = GameObjectUtility.FindByPath(null, "a");
        Transform b  = GameObjectUtility.FindByPath(a, "b");
        Transform c0 = GameObjectUtility.FindByPath(b, "c0");
        Transform c1 = GameObjectUtility.FindByPath(b, "c1");
        Transform d = GameObjectUtility.FindByPath(null, "a/b/c0/d");
        
        Assert.AreEqual(a,  created_a);
        Assert.AreEqual(b,  created_b);
        Assert.AreEqual(c0, created_c0);
        Assert.AreEqual(c1, created_c1);
        Assert.AreEqual(d,  created_d);
    }


//----------------------------------------------------------------------------------------------------------------------        
    
    [Test]
    public void FindOrCreateGameObjectsByPath() {

        Transform created_d  = GameObjectUtility.FindOrCreateByPath(null, "a/b/c0/d");
        Transform created_c0 = GameObjectUtility.FindOrCreateByPath(null, "a/b/c0");
        Transform created_c1 = GameObjectUtility.FindOrCreateByPath(null, "a/b/c1");
        Transform created_b  = GameObjectUtility.FindOrCreateByPath(null, "a/b");
        Transform created_a  = GameObjectUtility.FindOrCreateByPath(null, "a");        

        Transform a  = VerifySingleRootGameObjectExists("a");
        Transform b  = VerifySingleChildExists(a,"b");
        Transform c0 = VerifySingleChildExists(b,"c0");
        Transform c1 = VerifySingleChildExists(b,"c1");
        Transform d  = VerifySingleChildExists(c0,"d");
                
        Assert.AreEqual(a,  created_a);
        Assert.AreEqual(b,  created_b);
        Assert.AreEqual(c0, created_c0);
        Assert.AreEqual(c1, created_c1);
        Assert.AreEqual(d,  created_d);        
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    
    
    private Transform FindOrCreateGameObject(Transform parent, string path) {

        int pos = path.IndexOf('/');        
        if (pos == -1) {
            return FindOrCreateChild(parent, path);
        } else {
            Transform t = FindOrCreateChild(parent, path.Substring(0, pos));
            string remainingPath = path.Substring(pos + 1);
            return FindOrCreateGameObject(t, remainingPath);
        }
    }

    private Transform FindOrCreateChild(Transform parent, string childName) {

        Transform t = null == parent ? GameObjectUtility.FindFirstRoot(childName) : parent.Find(childName);
        if (null != t)
            return t;
            
        GameObject go = new GameObject(childName);
        t = go.transform;
        
        if (null == parent)
            return t;

        t.SetParent(parent);        
        return t;
    }

//----------------------------------------------------------------------------------------------------------------------    

    //Find root GameObjects with a certain anme
    private Transform VerifySingleRootGameObjectExists(string objectName) {

        Transform ret = null;
        int found = 0;
        GameObject[] roots = SceneManager.GetActiveScene().GetRootGameObjects();
        foreach (GameObject go in roots) {
            if (go.name != objectName) 
                continue;

            ++found;
            ret = go.transform;
        }
        
        Assert.AreEqual(1,found);
        Assert.IsNotNull(ret);
        return ret;
    }

    //Verify that only one child exists with a certain name
    private Transform VerifySingleChildExists(Transform parent, string childName) {
        Assert.IsNotNull(parent);

        Transform ret   = null;
        int       found = 0;

        int childCount = parent.childCount;
        for (int i = 0; i < childCount; ++i) {
            Transform curChild = parent.GetChild(i);
            if (curChild.name != childName)
                continue;

            ++found;
            ret = curChild.transform;
        }
        
        Assert.AreEqual(1,found);
        Assert.IsNotNull(ret);
        return ret;
    }
    
}

} //end namespace
