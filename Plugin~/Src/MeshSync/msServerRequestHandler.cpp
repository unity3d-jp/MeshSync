#include "pch.h"
#include "msServerRequestHandler.h"

#include "MeshSync/msMisc.h" //StartsWith()

#include "MeshSync/msServer.h"
#include "MeshSync/Utility/msNetworkUtility.h" //IsInLocalNetwork()

namespace ms {

using namespace Poco::Net;

class ServerRequestHandler : public HTTPRequestHandler {
public:
    ServerRequestHandler(Server *server);
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

private:
    Server *m_server = nullptr;
};

ServerRequestHandler::ServerRequestHandler(Server *server) : m_server(server) {
}

void ServerRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    //check the source connection
    if (!m_server->IsPublicAccessAllowed()) {

        //const IPAddress& ipAddress = request.clientAddress().host(); //This one doesn't represent the real source
        const std::string& hostAndPort = request.getHost();

        //Can't prevent DNS rebinding
        //const SocketAddress hostSocket (hostAndPort);
        //const IPAddress& ipAddress = hostSocket.host();
        //const bool isLoopback = ipAddress.isLoopback(); 
        //const bool isSiteLocal = ipAddress.isSiteLocal();
        //
        const bool isLocal = NetworkUtils::IsInLocalNetwork(hostAndPort);
        if (!isLocal) {
            m_server->serveText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
            return;
        }
    }

    if (!m_server->isServing()) {
        m_server->serveText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
        return;
    }

    const std::string& uri = request.getURI();
    if (uri == "set") {
        m_server->recvSet(request, response);
    }
    else if (uri == "delete") {
        m_server->recvDelete(request, response);
    }
    else if (uri == "fence") {
        m_server->recvFence(request, response);
    }
    else if (uri == "get") {
        m_server->recvGet(request, response);
    }
    else if (uri == "query") {
        m_server->recvQuery(request, response);
    }
    else if (uri == "text" || StartsWith(uri, "/text")) {
        m_server->recvText(request, response);
    }
    else if (StartsWith(uri, "/screenshot")) {
        m_server->recvScreenshot(request, response);
    }
    else if (StartsWith(uri, "/poll")) {
        m_server->recvPoll(request, response);
    }
    else if (StartsWith(uri, "/protocol_version")) {
        static const auto res = std::to_string(msProtocolVersion);
        m_server->serveText(response, res.c_str());
    }
    else if (StartsWith(uri, "/plugin_version")) {
        m_server->serveText(response, msPluginVersionStr);
    }
    else if (StartsWith(uri, "request_properties")) {
        m_server->recvServerInitiatedRequest(request, response);
    }
    else {
        // note: Poco handles commas in URI
        // e.g. "hoge/hage/../hige" -> "hoge/hige"
        //      "../../secret_file" -> "/"
        m_server->serveFiles(response, uri);
    }
}

//--------------------------------------------------------------------------------------------------------------------- 

ServerRequestHandlerFactory::ServerRequestHandlerFactory(Server *server): m_server(server) {
}

HTTPRequestHandler* ServerRequestHandlerFactory::createRequestHandler(const HTTPServerRequest&) {
    return new ServerRequestHandler(m_server);
}



} // namespace ms

