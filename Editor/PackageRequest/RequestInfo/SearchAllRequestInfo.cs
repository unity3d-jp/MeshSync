using System;                               //Action
using UnityEditor.PackageManager.Requests;  //Request<T>
using UnityEditor.PackageManager;           //PackageInfo

namespace Unity.AnimeToolbox {

internal class SearchAllRequestInfo {
    internal readonly bool OfflineMode;
    internal readonly Action<Request<PackageInfo[]>> OnSuccessAction;
    internal readonly Action<Request<PackageInfo[]>> OnFailAction;

    internal SearchAllRequestInfo(bool offlineMode, 
        Action<Request<PackageInfo[]>> onSuccess, Action<Request<PackageInfo[]>> onFail)
    {
        OfflineMode = offlineMode;
        OnSuccessAction = onSuccess;
        OnFailAction = onFail;
    }
}

} //namespace Unity.AnimeToolbox
