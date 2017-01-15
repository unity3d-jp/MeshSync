#include "pch.h"
#include "msClient.h"

namespace ms {

using namespace Poco;
using namespace Poco::Net;

Client::Client(const ClientSettings & settings)
    : m_settings(settings)
{
}

bool Client::send(const DeleteData& data)
{
    const DeleteData *a[] = { &data };
    return send(a, 1);
}
bool Client::send(const DeleteData * const data[], int num)
{
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_POST, "delete" };
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

bool Client::send(const MeshData& data)
{
    const MeshData *a[] = {&data};
    return send(a, 1);
}

bool Client::send(const MeshData * const data[], int num)
{
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_POST, "mesh" };
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

Client::DaraList Client::send(const GetData& gdata)
{
    DaraList ret;
    try {
        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(5000 * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_POST, "get" };
        request.setContentType("application/octet-stream");

        {
            request.setContentLength(gdata.getSerializeSize());
            auto& os = session.sendRequest(request);
            gdata.serialize(os);
        }

        HTTPResponse response;
        auto& is = session.receiveResponse(response);

        int num_data = 0;
        is.read((char*)&num_data, 4);
        for (int i = 0; i < num_data; ++i) {
            auto tmp = new MeshData();
            tmp->deserialize(is);
            ret.emplace_back(tmp);
        }
    }
    catch (...) {
    }
    return ret;
}

} // namespace ms
