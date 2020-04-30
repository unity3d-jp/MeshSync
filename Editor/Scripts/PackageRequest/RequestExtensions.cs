using UnityEditor.PackageManager.Requests;  //ListRequest
using UnityEditor.PackageManager;       //PackageCollection  
using System.Collections.Generic;       //IEnumerable


namespace Unity.AnimeToolbox {

/// <summary>
/// An extension class to extend the functionality of UnityEditor.PackageManager.Requests classes
/// </summary>
internal static class RequestExtensions
{
    /// <summary>
    /// Find a PackageInfo which has the passed parameter 
    /// </summary>
    /// <param name="listRequest">this Request object</param>
    /// <param name="packageName">the package name</param>
    /// <returns>The PackageInfo if found, otherwise null</returns>
    /// 
    public static PackageInfo FindPackage(this Request<PackageCollection> listRequest, string packageName) {
        IEnumerable<PackageInfo> packageInfoCollection = listRequest.Result as IEnumerable<PackageInfo>;
        if (null == packageInfoCollection) {
            return null;
        }

        using (var enumerator = packageInfoCollection.GetEnumerator()) {

            while (enumerator.MoveNext()) {
                PackageInfo curInfo = enumerator.Current;
                if (null == curInfo)
                    continue;
                if (curInfo.name == packageName) {
                    return curInfo;
                }
            }
        }

        return null;
    }
}

} //namespace Unity.AnimeToolbox
