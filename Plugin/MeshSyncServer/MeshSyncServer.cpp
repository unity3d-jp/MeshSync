#include "pch.h"
#include "MeshSyncServer.h"


namespace mss {

using namespace Poco::Net;


class RequestHandler : public HTTPRequestHandler
{
public:
    RequestHandler(const std::string& uri);
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override;

private:
};

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &request) override;
};



RequestHandler::RequestHandler(const std::string& uri)
{
}

void RequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
}

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest &request)
{
    return new RequestHandler(request.getURI());
}



Server::Server(const ServerSettings& settings)
    : m_settings(settings)
{
}

Server::~Server()
{
    stop();
}

bool Server::start()
{
    if (!m_server) {
        m_end_flag = false;

        auto* params = new Poco::Net::HTTPServerParams;
        params->setMaxQueued(m_settings.max_queue);
        params->setMaxThreads(m_settings.max_threads);
        params->setThreadIdleTime(Poco::Timespan(3, 0));

        try {
            Poco::Net::ServerSocket svs(m_settings.port);
            m_server = new Poco::Net::HTTPServer(new RequestHandlerFactory(), svs, params);
            m_server->start();
        }
        catch (Poco::IOException &e) {
            //wiDebugPrint(e.what());
            //wiAssert(false);
            return false;
        }
    }

    return true;
}

bool Server::stop()
{
    return false;
}

} // namespace mss


extern "C" {

msAPI mss::Server* mssStartServer(const mss::ServerSettings *settings)
{
    mss::Server *ret = nullptr;
    if (settings) {
        ret = new mss::Server(*settings);
        if (!ret->start()) {
            delete ret;
            ret = nullptr;
        }
    }
    return ret;
}

msAPI void  mssStopServer(mss::Server *server)
{
    delete server;
}

} // extern "C"
