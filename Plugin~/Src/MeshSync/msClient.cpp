#include "pch.h"
#include "MeshSync/msClient.h"
#include "MeshSync/SceneGraph/msScene.h" //Scene
#include "MeshSync/SceneGraph/msCurve.h"

namespace ms {

using namespace Poco;
using namespace Poco::Net;

HTTPClientSession* m_properties_session;

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
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_GET, "/protocol_version" };
        session.sendRequest(request);

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        std::ostringstream ostr;
        StreamCopier::copyStream(rs, ostr);
        auto content = ostr.str();
        if (response.getStatus() != HTTPResponse::HTTP_OK) {
            m_error_message = "Server is not working.";
        }
        else {
            if (std::atoi(content.c_str()) == msProtocolVersion) {
                m_error_message.clear();
                return true;
            }
            else {
                m_error_message = "Version doesn't match server.";
            }
        }
    }
    catch (const Poco::TimeoutException& /*e*/) {
        // in this case e.what() is empty.
        m_error_message = "Could not reach server (timeout).";
    }
    catch (const Poco::Exception& e) {
        m_error_message = e.what();
    }

    if (!m_error_message.empty()) {
        char buf[512];
        sprintf(buf, " [%s:%d]", m_settings.server.c_str(), (int)m_settings.port);
        m_error_message += buf;
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
                ret = Scene::create(is);
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

void Client::abortPropertiesRequest() {
    if (m_properties_session) {
        m_properties_session->abort();
        m_properties_session = nullptr;
    }
}

bool Client::send(const ServerInitiatedMessage& mes)
{
    try {
        
        HTTPClientSession session{ m_settings.server, m_settings.port };

        m_properties_session = &session;

        session.setTimeout(Poco::Timespan()); // infinite timeout, this is cancelled manually.
         
        HTTPRequest request{ HTTPRequest::HTTP_POST, "request_properties" };
        request.setContentType("application/octet-stream");
        request.setExpectContinue(true);
        request.setContentLength(ssize(mes));
        auto& os = session.sendRequest(request);
        mes.serialize(os);
        os.flush();

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);

        auto reqResponse = ServerInitiatedMessageResponse();
        reqResponse.deserialize(rs);

        properties = std::vector<PropertyInfo>();
        for (int i = 0; i < reqResponse.properties.size(); i++)
        {
            auto prop = PropertyInfo(reqResponse.properties[i]);
            reqResponse.properties[i].data.share(prop.data);
            properties.push_back(prop);
        }

        entities.clear();
        entities.insert(entities.end(), reqResponse.entities.begin(), reqResponse.entities.end());
       
       /* curves = std::vector<CurvePtr>();
        for (int i = 0; i < reqResponse.curves.size(); i++)
        {
            curves.push_back(reqResponse.curves[i]);
        }

        meshes = std::vector<MeshPtr>();
        for (int i = 0; i < reqResponse.meshes.size(); i++)
        {
            meshes.push_back(reqResponse.meshes[i]);
        }*/

        messageFromServer = reqResponse.message;

        m_properties_session = nullptr;

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
            if (response.getStatus() == HTTPResponse::HTTP_OK) {
                ret.reset(new ResponseMessage());
                ret->deserialize(is);
            }
            else {
                m_error_message = "Server is stopped.";
            }
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
