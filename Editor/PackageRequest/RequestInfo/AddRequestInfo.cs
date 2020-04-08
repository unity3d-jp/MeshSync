using System;                               //Action
using UnityEditor.PackageManager.Requests;  //ListRequest, AddRequest, etc
using UnityEditor.PackageManager;           //PackageInfo

namespace Unity.AnimeToolbox {

class AddRequestInfo {
    internal string PackageName;
    internal Action<Request<PackageInfo>> OnSuccessAction;
    internal Action<Request<PackageInfo>> OnFailAction;
    internal AddRequestInfo(string packageName,
        Action<Request<PackageInfo>> onSuccess, Action<Request<PackageInfo>> onFail)
    {
        PackageName = packageName;
        OnSuccessAction = onSuccess;
        OnFailAction = onFail;
    }
}

} //namespace Unity.AnimeToolbox
