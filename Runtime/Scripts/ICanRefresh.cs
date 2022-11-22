namespace Unity.MeshSync {

//[TODO-sin: 2022-11-21] Move to FIU
internal interface ICanRefresh {
        
    bool Refresh(); //return false if Refresh() failed, true otherwise
}

} //end namespace