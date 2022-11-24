#include "pch.h"
#include "TestUtility.h"
#include "Test.h"               //GetArg
#include "MeshSync/AsyncSceneSender.h" //ms::AsyncSceneSender

ms::ClientSettings TestUtility::GetClientSettings(){
    ms::ClientSettings ret;
    GetArg("server", ret.server);
    int port;
    if (GetArg("port", port))
        ret.port = (uint16_t)port;
    return ret;
}

//----------------------------------------------------------------------------------------------------------------------

void TestUtility::Send(ms::ScenePtr scene) {
    ms::AsyncSceneSender sender;

    // Set to invalid ID so it does not trigger popup for server:
    sender.session_id = ms::InvalidID;

    sender.client_settings = GetClientSettings();
    if (sender.isServerAvailable()) {
        sender.add(scene);
        sender.kick();
    }
}
