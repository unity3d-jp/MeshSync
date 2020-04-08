using System;                               //Action
using UnityEditor.PackageManager.Requests;  //Request<T>
using UnityEditor.PackageManager;           //PackageInfo

namespace Unity.AnimeToolbox {

class SearchRequestInfo {
    internal string PackageName;
    internal bool OfflineMode;
    internal Action<Request<PackageInfo[]>> OnSuccessAction;
    internal Action<Request<PackageInfo[]>> OnFailAction;

    internal SearchRequestInfo(string packageName, bool offlineMode,
        Action<Request<PackageInfo[]>> onSuccess, Action<Request<PackageInfo[]>> onFail)
    {
        PackageName = packageName;
        OfflineMode = offlineMode;
        OnSuccessAction = onSuccess;
        OnFailAction = onFail;
    }
}

} //namespace Unity.AnimeToolbox
