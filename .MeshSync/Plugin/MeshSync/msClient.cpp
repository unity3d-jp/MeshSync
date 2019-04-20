#include "pch.h"
#include "msClient.h"

namespace ms {

using namespace Poco;
using namespace Poco::Net;

Client::Client(const ClientSettings & settings)
    : m_settings(settings)
{
}

const std::string& Client::getErrorMessage() const
{
    return m_error_message;
}

bool Client::isServerAvailable(int timeout_ms)
{
    QueryMessage query;
    query.query_type = QueryMessage::QueryType::ProtocolVersion;
    auto response = send(query, timeout_ms);
    if (response) {
        if (!response->text.empty() && std::atoi(response->text[0].c_str()) == msProtocolVersion) {
            return true;
        }
        else {
            m_error_message = "Version doesn't match server.";
        }
    }
    return false;
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

bool Client::send(const FenceMessage& mes)
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

ResponseMessagePtr Client::send(const QueryMessage& mes, int timeout_ms)
{
    ResponseMessagePtr ret;
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(timeout_ms * 1000);

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
    catch (const Poco::TimeoutException& /*e*/) {
        // in this case e.what() is empty.
        m_error_message = "Could not reach server (timeout).";
    }
    catch (const Poco::Exception& e) {
        m_error_message = e.what();
    }
    return ret;
}

ResponseMessagePtr Client::send(const QueryMessage& mes)
{
    return send(mes, m_settings.timeout_ms);
}

} // namespace ms
