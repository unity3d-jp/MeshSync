#pragma once

#ifdef _WIN32
    #define msAPI __declspec(dllexport)
#else
    #define msAPI
#endif

namespace mss {

struct ServerSettings
{
    int max_queue = 100;
    int max_threads = 4;
    uint16_t port = 8080;
};

class Server
{
public:
    Server(const ServerSettings& settings);
    ~Server();
    bool start();
    bool stop();

private:
    ServerSettings m_settings;
    Poco::Net::HTTPServer *m_server = nullptr;
    bool m_end_flag = false;
};

} // namespace mss


extern "C" {

msAPI mss::Server*  mssStartServer(const mss::ServerSettings *settings);
msAPI void          mssStopServer(mss::Server *server);

} // extern "C"
