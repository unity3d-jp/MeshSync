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
    const EventData *a[] = {&data};
    return send(a, 1);
}

bool Client::send(const EventData * const data[], int num)
{
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_POST, "event" };
        request.setContentType("application/octet-stream");

        size_t len = 4;
        for (int i = 0; i < num; ++i) {
            len += data[i]->getSerializeSize();
        }

        request.setContentLength(len);
        auto& os = session.sendRequest(request);
        os.write((char*)&num, 4);
        for (int i = 0; i < num; ++i) {
            data[i]->serialize(os);
        }

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
