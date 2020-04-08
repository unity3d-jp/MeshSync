using System;                               //Action
using UnityEditor.PackageManager.Requests;  //ListRequest, AddRequest, etc
using UnityEditor.PackageManager;           //PackageCollection

namespace Unity.AnimeToolbox {
internal class ListRequestInfo {
    internal readonly bool m_offlineMode;
    internal readonly bool m_includeIndirectIndependencies;
    internal readonly Action<Request<PackageCollection>> m_onSuccessAction;
    internal readonly Action<Request<PackageCollection>> m_onFailAction;

    internal ListRequestInfo(bool offlineMode, bool includeIndirectDependencies,
        Action<Request<PackageCollection>> onSuccess, Action<Request<PackageCollection>> onFail)
    {
        m_offlineMode = offlineMode;
        m_includeIndirectIndependencies = includeIndirectDependencies;
        m_onSuccessAction = onSuccess;
        m_onFailAction = onFail;
    }
}

} //namespace Unity.AnimeToolbox
