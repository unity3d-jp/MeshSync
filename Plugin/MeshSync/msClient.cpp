#include "pch.h"
#include "msClient.h"

namespace ms {

using namespace Poco;
using namespace Poco::Net;

Client::Client(const ClientSettings & settings)
    : m_settings(settings)
{
}

ScenePtr Client::send(const GetMessage& mes)
{
    ScenePtr ret;
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        {
            HTTPRequest request{ HTTPRequest::HTTP_POST, "get" };
            request.setContentType("application/octet-stream");
            request.setExpectContinue(true);
            request.setContentLength(ssize(mes));
            auto& os = session.sendRequest(request);
            mes.serialize(os);
            os.flush();
        }

        {
            HTTPResponse response;
            auto& is = session.receiveResponse(response);
            try {
                ret.reset(new Scene());
                ret->deserialize(is);
            }
            catch (const std::exception&) {
                ret.reset();
            }
        }
    }
    catch (...) {
    }
    return ret;
}

bool Client::send(const SetMessage& mes)
{
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_POST, "set" };
        request.setContentType("application/octet-stream");
        request.setExpectContinue(true);
        request.setContentLength(ssize(mes));
        auto& os = session.sendRequest(request);
        mes.serialize(os);
        os.flush();

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        std::ostringstream ostr;
        StreamCopier::copyStream(rs, ostr);
        return response.getStatus() == HTTPResponse::HTTP_OK;
    }
    catch (...) {
        return false;
    }
}

bool Client::send(const DeleteMessage& mes)
{
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_POST, "delete" };
        request.setContentType("application/octet-stream");
        request.setExpectContinue(true);
        request.setContentLength(ssize(mes));
        auto& os = session.sendRequest(request);
        mes.serialize(os);
        os.flush();

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        std::ostringstream ostr;
        StreamCopier::copyStream(rs, ostr);
        return response.getStatus() == HTTPResponse::HTTP_OK;
    }
    catch (...) {
        return false;
    }
}

bool Client::send(const FenceMessage & mes)
{
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_POST, "fence" };
        request.setContentType("application/octet-stream");
        request.setExpectContinue(true);
        request.setContentLength(ssize(mes));
        auto& os = session.sendRequest(request);
        mes.serialize(os);
        os.flush();

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        std::ostringstream ostr;
        StreamCopier::copyStream(rs, ostr);
        return response.getStatus() == HTTPResponse::HTTP_OK;
    }
    catch (...) {
        return false;
    }
}

MessagePtr Client::send(const QueryMessage & mes)
{
    MessagePtr ret;
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(5000 * 1000);

        {
            HTTPRequest request{ HTTPRequest::HTTP_POST, "query" };
            request.setContentType("application/octet-stream");
            request.setExpectContinue(true);
            request.setContentLength(ssize(mes));
            auto& os = session.sendRequest(request);
            mes.serialize(os);
            os.flush();
        }

        {
            HTTPResponse response;
            auto& is = session.receiveResponse(response);
            ret.reset(new ResponseMessage());
            ret->deserialize(is);
        }
    }
    catch (...) {
    }
    return ret;
}

} // namespace ms
