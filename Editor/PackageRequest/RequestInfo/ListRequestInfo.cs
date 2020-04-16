using System;                               //Action
using UnityEditor.PackageManager.Requests;  //ListRequest, AddRequest, etc
using UnityEditor.PackageManager;           //PackageCollection

namespace Unity.AnimeToolbox {
internal class ListRequestInfo {
    internal readonly bool OfflineMode;
    internal readonly bool IncludeIndirectIndependencies;
    internal readonly Action<Request<PackageCollection>> OnSuccessAction;
    internal readonly Action<Request<PackageCollection>> OnFailAction;

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
