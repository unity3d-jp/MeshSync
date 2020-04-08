using System;                               //Action

namespace Unity.AnimeToolbox {
internal class RemoveRequestInfo{
    internal readonly string m_packageName;
    internal readonly Action m_onSuccessAction;
    internal readonly Action m_onFailAction;

    internal RemoveRequestInfo(string packageName,
        Action onSuccess, Action onFail)
    {
        m_packageName = packageName;
        m_onSuccessAction = onSuccess;
        m_onFailAction = onFail;
    }
}

} //namespace Unity.AnimeToolbox
