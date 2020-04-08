using System;                               //Action
using UnityEditor.PackageManager.Requests;  //Request<T>
using UnityEditor.PackageManager;           //PackageInfo

namespace Unity.AnimeToolbox {
    
internal class SearchRequestInfo {
    internal readonly string m_packageName;
    internal readonly bool m_offlineMode;
    internal readonly Action<Request<PackageInfo[]>> m_onSuccessAction;
    internal readonly Action<Request<PackageInfo[]>> m_onFailAction;

    internal SearchRequestInfo(string packageName, bool offlineMode,
        Action<Request<PackageInfo[]>> onSuccess, Action<Request<PackageInfo[]>> onFail)
    {
        m_packageName = packageName;
        m_offlineMode = offlineMode;
        m_onSuccessAction = onSuccess;
        m_onFailAction = onFail;
    }
}

} //namespace Unity.AnimeToolbox
