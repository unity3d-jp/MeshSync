using UnityEditor.PackageManager.Requests;  //Request
using UnityEditor.PackageManager;           //StatusCode
using System;                               //Action

namespace Unity.AnimeToolbox {

//---------------------------------------------------------------------------------------------------------------------
//Non-generics version
internal class RequestJob : IRequestJob  {

    internal RequestJob(Request req, Action onSuccess, Action onFail) {
        m_request = req;
        m_onSuccess = onSuccess;
        m_onFail = onFail;
    }

//---------------------------------------------------------------------------------------------------------------------

    public StatusCode Update() {
        if (null == m_request) {
            OnFail();          
            return StatusCode.Failure;
        }

        if (!m_request.IsCompleted) 
            return StatusCode.InProgress;
        
        if (StatusCode.Success == m_request.Status ) {
            OnSuccess();          
            return StatusCode.Success;
        } else {
            OnFail();          
            return StatusCode.Failure;
        }

    }

//---------------------------------------------------------------------------------------------------------------------


    void OnSuccess() {
        m_onSuccess?.Invoke();
    }

//---------------------------------------------------------------------------------------------------------------------

    void OnFail() {
        m_onFail?.Invoke();
    }

    readonly Request m_request;
    readonly Action m_onSuccess;
    readonly Action m_onFail;
} //end RequestJob (non-generics version)

//---------------------------------------------------------------------------------------------------------------------

//RequestJob (generics version)
//Examples of T: PackageCollection(from ListRequest), PackageInfo (from AddRequest)
internal class RequestJob<T> : IRequestJob  {

    internal RequestJob(Request<T> req, Action<Request<T>> onSuccess, Action<Request<T>> onFail) {
        m_request = req;
        m_onSuccess = onSuccess;
        m_onFail = onFail;
    }

//---------------------------------------------------------------------------------------------------------------------

    public StatusCode Update() {
        if (null == m_request) {
            OnFail();          
            return StatusCode.Failure;
        }

        if (!m_request.IsCompleted) 
            return StatusCode.InProgress;

        if (StatusCode.Success == m_request.Status ) {
            OnSuccess();          
            return StatusCode.Success;
        } else {
            OnFail();          
            return StatusCode.Failure;
        }

    }

//---------------------------------------------------------------------------------------------------------------------


    void OnSuccess() {
        m_onSuccess?.Invoke(m_request);
    }

//---------------------------------------------------------------------------------------------------------------------

    void OnFail() {
        m_onFail?.Invoke(m_request);
    }

//---------------------------------------------------------------------------------------------------------------------

    readonly Request<T> m_request;
    readonly Action<Request<T>> m_onSuccess;
    readonly Action<Request<T>> m_onFail;
}

} //Unity.AnimeToolbox
