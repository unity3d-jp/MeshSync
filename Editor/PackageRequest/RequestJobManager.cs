using System.Collections.Generic;           //HashSet
using UnityEditor.PackageManager.Requests;  //ListRequest, AddRequest, etc
using UnityEditor.PackageManager;           //PackageCollection
using System;                               //Action


namespace Unity.AnimeToolbox {


/// <summary>
/// An editor class to manage requests to UnityEditor.PackageManager.Client
/// This class will perform its operations in background while Unity is running.
/// </summary>
public static class RequestJobManager 
{
    [UnityEditor.InitializeOnLoadMethod]
    static void OnLoad() {    
        UnityEditor.EditorApplication.update+=UpdateRequestJobs;        
    }

//---------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Queue a job to list the packages the project depends on.
    /// </summary>
    /// <param name="offlineMode">Specifies whether or not the Package Manager requests the latest information about
    ///     the project's packages from the remote Unity package registry. When offlineMode is true,
    ///     the PackageInfo objects in the PackageCollection returned by the Package Manager contain information
    ///     obtained from the local package cache, which could be out of date.</param>
    /// <param name="includeIndirectIndependencies">Set to true to include indirect dependencies in the
    ///     PackageCollection returned by the Package Manager. Indirect dependencies include packages referenced
    ///     in the manifests of project packages or in the manifests of other indirect dependencies. Set to false
    ///     to include only the packages listed directly in the project manifest.</param>
    /// <param name="onSuccess">Action which is executed if the request succeeded</param>
    /// <param name="onFail">Action which is executed if the request failed </param>
    /// 
    public static void CreateListRequest(bool offlineMode, bool includeIndirectIndependencies,
       Action<Request<PackageCollection>> onSuccess, Action<Request<PackageCollection>> onFail)
    {
        m_pendingListRequests.Enqueue(new ListRequestInfo(offlineMode, includeIndirectIndependencies, onSuccess, onFail));
    }

//---------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Queue a job to add a package dependency to the project.
    /// </summary>
    /// <param name="packageName">The name or ID of the package to add. If only the name is specified,
    ///     the latest version of the package is installed.</param>
    /// <param name="onSuccess">Action which is executed if the request succeeded</param>
    /// <param name="onFail">Action which is executed if the request failed </param>
    /// 
    public static void CreateAddRequest(string packageName,
        Action<Request<PackageInfo>> onSuccess, Action<Request<PackageInfo>> onFail)
    {
        m_pendingAddRequests.Enqueue(new AddRequestInfo(packageName, onSuccess, onFail));
    }

//---------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Queue a job to removes a previously added package from the project.
    /// </summary>
    /// <param name="packageName">The name or ID of the package to add. </param>
    /// <param name="onSuccess">Action which is executed if the request succeeded</param>
    /// <param name="onFail">Action which is executed if the request failed </param>
    /// 
    public static void CreateRemoveRequest(string packageName, Action onSuccess, Action onFail)
    {
        m_pendingRemoveRequests.Enqueue(new RemoveRequestInfo(packageName, onSuccess, onFail));
    }

//---------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Queue a job to searches the Unity package registry for the given package.
    /// </summary>
    /// <param name="packageName">The name or ID of the package to add.</param>
    /// <param name="offlineMode">Specifies whether or not the Package Manager requests the latest information about
    ///     the project's packages from the remote Unity package registry. When offlineMode is true,
    ///     the PackageInfo objects in the PackageCollection returned by the Package Manager contain information
    ///     obtained from the local package cache, which could be out of date.</param>
    /// <param name="onSuccess">Action which is executed if the request succeeded</param>
    /// <param name="onFail">Action which is executed if the request failed </param>
    /// 
    public static void CreateSearchRequest(string packageName, bool offlineMode,
        Action<Request<PackageInfo[]>> onSuccess, Action<Request<PackageInfo[]>> onFail)
    {
        m_pendingSearchRequests.Enqueue(new SearchRequestInfo(packageName, offlineMode, onSuccess, onFail));
    }

//---------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Queue a job to search the Unity package registry for all packages compatible with the current Unity version.
    /// </summary>
    /// <param name="offlineMode">Specifies whether or not the Package Manager requests the latest information about
    ///     the project's packages from the remote Unity package registry. When offlineMode is true,
    ///     the PackageInfo objects in the PackageCollection returned by the Package Manager contain information
    ///     obtained from the local package cache, which could be out of date.</param>
    /// <param name="onSuccess">Action which is executed if the request succeeded</param>
    /// <param name="onFail">Action which is executed if the request failed </param>
    /// 
    public static void CreateSearchAllRequest(bool offlineMode,
        Action<Request<PackageInfo[]>> onSuccess, Action<Request<PackageInfo[]>> onFail)
    {
        m_pendingSearchAllRequests.Enqueue(new SearchAllRequestInfo(offlineMode, onSuccess, onFail));
    }
   
//---------------------------------------------------------------------------------------------------------------------   

    static void UpdateRequestJobs()
    {
        //Process pending list requests
        using (var enumerator = m_pendingListRequests.GetEnumerator()) {
            while (enumerator.MoveNext()) {
                ListRequestInfo info = enumerator.Current;
                if (null == info)
                    continue;
                ListRequest listReq = Client.List(info.m_offlineMode, info.m_includeIndirectIndependencies);
                m_requestJobs.Add(new RequestJob<PackageCollection>(listReq,info.m_onSuccessAction,info.m_onFailAction));               
            }
            m_pendingListRequests.Clear();
        }

        //Process pending add requests
        using (var enumerator = m_pendingAddRequests.GetEnumerator()) {   
            while (enumerator.MoveNext()) {
                AddRequestInfo info = enumerator.Current;
                if (null == info)
                    continue;
                AddRequest addReq = Client.Add(info.m_packageName);
                m_requestJobs.Add(new RequestJob<PackageInfo>(addReq,info.m_onSuccessAction,info.m_onFailAction));
            }
            m_pendingAddRequests.Clear();
        }

        //Process pending RemoveRequests
        using (var enumerator = m_pendingRemoveRequests.GetEnumerator()) 
        {   
            while (enumerator.MoveNext()) {
                RemoveRequestInfo info = enumerator.Current;
                if (null == info)
                    continue;
                RemoveRequest removeReq = Client.Remove(info.m_packageName);
                m_requestJobs.Add(new RequestJob(removeReq,info.m_onSuccessAction,info.m_onFailAction));
            }
            m_pendingRemoveRequests.Clear();
        }

        //Process pending SearchRequests
        using (var enumerator = m_pendingSearchRequests.GetEnumerator()) 
        {   
            while (enumerator.MoveNext()) {
                SearchRequestInfo info = enumerator.Current;
                if (null == info)
                    continue;
                SearchRequest searchReq = Client.Search(info.m_packageName, info.m_offlineMode);
                m_requestJobs.Add(new RequestJob<PackageInfo[]>(searchReq,info.m_onSuccessAction,info.m_onFailAction));
            }
            m_pendingSearchRequests.Clear();
        }

        //Process pending SearchAllRequests
        using (var enumerator = m_pendingSearchAllRequests.GetEnumerator()) { 
            while (enumerator.MoveNext()) {
                SearchAllRequestInfo info = enumerator.Current;
                if (null == info)
                    continue;
                SearchRequest searchReq = Client.SearchAll(info.m_offlineMode);
                m_requestJobs.Add(new RequestJob<PackageInfo[]>(searchReq,info.m_onSuccessAction,info.m_onFailAction));
            }
            m_pendingSearchAllRequests.Clear();
        }

        //Update and register completed jobs
        using (var enumerator = m_requestJobs.GetEnumerator()) {
            while (enumerator.MoveNext()) {
                if (null == enumerator.Current)
                    continue;
                
                StatusCode code = enumerator.Current.Update();
                if (StatusCode.Failure == code || StatusCode.Success == code) {
                    m_jobsToDelete.Add(enumerator.Current);
                }
            }
        }

        //delete completed jobs
        using (var enumerator = m_jobsToDelete.GetEnumerator()) { 
            while (enumerator.MoveNext()) {
                m_requestJobs.Remove(enumerator.Current);
            }
            m_jobsToDelete.Clear();
            enumerator.Dispose();
        }

    }

//---------------------------------------------------------------------------------------------------------------------

    static readonly Queue<ListRequestInfo> m_pendingListRequests = new Queue<ListRequestInfo>();
    static readonly Queue<AddRequestInfo>  m_pendingAddRequests = new Queue<AddRequestInfo>();
    static readonly Queue<RemoveRequestInfo>  m_pendingRemoveRequests = new Queue<RemoveRequestInfo>();
    static readonly Queue<SearchRequestInfo>  m_pendingSearchRequests = new Queue<SearchRequestInfo>();
    static readonly Queue<SearchAllRequestInfo>  m_pendingSearchAllRequests = new Queue<SearchAllRequestInfo>();

    static readonly HashSet<IRequestJob> m_requestJobs = new HashSet<IRequestJob>();
    static readonly List<IRequestJob> m_jobsToDelete = new List<IRequestJob>();

}

} //namespace Unity.AnimeToolbox 
