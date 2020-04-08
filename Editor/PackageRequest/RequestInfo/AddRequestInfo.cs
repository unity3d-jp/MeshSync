using System;                               //Action
using UnityEditor.PackageManager.Requests;  //ListRequest, AddRequest, etc
using UnityEditor.PackageManager;           //PackageInfo

namespace Unity.AnimeToolbox {

internal class AddRequestInfo {
    internal readonly string m_packageName;
    internal readonly Action<Request<PackageInfo>> m_onSuccessAction;
    internal readonly Action<Request<PackageInfo>> m_onFailAction;
    internal AddRequestInfo(string packageName,
        Action<Request<PackageInfo>> onSuccess, Action<Request<PackageInfo>> onFail)
    {
        m_packageName = packageName;
        m_onSuccessAction = onSuccess;
        m_onFailAction = onFail;
    }
}

} //namespace Unity.AnimeToolbox
