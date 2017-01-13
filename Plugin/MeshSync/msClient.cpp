#include "pch.h"
#include "msClient.h"

namespace ms {

using namespace Poco;
using namespace Poco::Net;

Client::Client(const ClientSettings & settings)
    : m_settings(settings)
{
}

bool Client::sendEdit(const EventData& data)
{
    const EventData *a[] = {&data};
    return sendEdit(a, 1);
}

bool Client::sendEdit(const EventData * const data[], int num)
{
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_POST, "edit" };
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

Client::DaraList Client::sendGet(GetFlags flags)
{
    DaraList data;
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_POST, "get" };
        request.setContentType("application/octet-stream");

        {
            GetData data;
            request.setContentLength(data.getSerializeSize());
            auto& os = session.sendRequest(request);
            data.serialize(os);
        }

        HTTPResponse response;
        auto& is = session.receiveResponse(response);

        int num_data = 0;
        is.read((char*)&num_data, 4);
        for (int i = 0; i < num_data; ++i) {
            EventType type;
            is.read((char*)&type, 4);

            switch (type) {
            case EventType::Xform:
            {
                auto tmp = new XformData();
                tmp->deserialize(is);
                data.emplace_back(tmp);
                break;
            }
            case EventType::Mesh:
            {
                auto tmp = new MeshData();
                tmp->deserialize(is);
                data.emplace_back(tmp);
                break;
            }
            }
        }
    }
    catch (...) {
    }
    return data;
}

} // namespace ms
