using System;                               //Action
using UnityEditor.PackageManager.Requests;  //ListRequest, AddRequest, etc
using UnityEditor.PackageManager;           //PackageCollection

namespace Unity.AnimeToolbox {

class ListRequestInfo {
    internal bool OfflineMode;
    internal bool IncludeIndirectIndependencies;
    internal Action<Request<PackageCollection>> OnSuccessAction;
    internal Action<Request<PackageCollection>> OnFailAction;

    internal ListRequestInfo(bool offlineMode, bool includeIndirectDependencies,
        Action<Request<PackageCollection>> onSuccess, Action<Request<PackageCollection>> onFail)
    {
        OfflineMode = offlineMode;
        IncludeIndirectIndependencies = includeIndirectDependencies;
        OnSuccessAction = onSuccess;
        OnFailAction = onFail;
    }
}

} //namespace Unity.AnimeToolbox
