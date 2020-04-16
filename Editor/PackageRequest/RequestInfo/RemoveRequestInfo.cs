using System;                               //Action

namespace Unity.AnimeToolbox {
internal class RemoveRequestInfo{
    internal readonly string PackageName;
    internal readonly Action OnSuccessAction;
    internal readonly Action OnFailAction;

    internal RemoveRequestInfo(string packageName,
        Action onSuccess, Action onFail)
    {
        PackageName = packageName;
        OnSuccessAction = onSuccess;
        OnFailAction = onFail;
    }
}

} //namespace Unity.AnimeToolbox
