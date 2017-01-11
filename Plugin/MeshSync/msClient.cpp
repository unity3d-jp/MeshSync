#include "pch.h"
#include "msClient.h"

namespace ms {

using namespace Poco;
using namespace Poco::Net;

Client::Client(const ClientSettings & settings)
    : m_settings(settings)
{
}

bool Client::sendEdit(const MeshData& data)
{
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };

        HTTPRequest request{ HTTPRequest::HTTP_POST, "mesh" };
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

} // namespace ms
