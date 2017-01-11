#include "pch.h"
#include "msClient.h"

namespace ms {

using namespace Poco;
using namespace Poco::Net;

Client::Client(const ClientSettings & settings)
    : m_settings(settings)
{
}

bool Client::send(const EventData& data)
{
    switch (data.type) {
    case EventType::Delete:
        sendDelete(static_cast<const DeleteData&>(data));
        break;
    case EventType::Xform:
        sendXform(static_cast<const XformData&>(data));
        break;
    case EventType::Mesh:
        sendMesh(static_cast<const MeshData&>(data));
        break;
    }
    return false;
}

template<class Data>
bool Client::send(const char *uri, const Data& data)
{
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        HTTPRequest request{ HTTPRequest::HTTP_POST, uri };
        request.setContentType("application/octet-stream");
        request.setContentLength(data.getSerializeSize());
        auto& os = session.sendRequest(request);
        data.serialize(os);

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        std::ostringstream ostr;
        StreamCopier::copyStream(rs, ostr);
        return true;
    }
    catch (...) {
        return false;
    }
}

bool Client::sendDelete(const DeleteData& data)
{
    return send("delete", data);
}

bool Client::sendXform(const XformData& data)
{
    return send("xform", data);
}

bool Client::sendMesh(const MeshData& data)
{
    return send("mesh", data);
}

} // namespace ms
