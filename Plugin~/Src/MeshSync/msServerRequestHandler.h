#pragma once

#include "Poco/Net/HTTPRequestHandlerFactory.h" //HTTPRequestHandlerFactory
#include "MeshSync/msProtocol.h"

namespace Poco {
    namespace Net {
        class HTTPServerRequest;
        class HTTPRequestHandler;
    }
}

//----------------------------------------------------------------------------------------------------------------------
namespace ms {

    class Server;

    class ServerRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        ServerRequestHandlerFactory(Server *server);
        Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

    private:
        Server *m_server = nullptr;
    };

} // namespace ms

