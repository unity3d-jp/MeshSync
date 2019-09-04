#include "pch.h"
#include "msGraphicsInterface.h"

#ifdef msEnableD3D11
#define DefPtr(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))
DefPtr(ID3D11Device);
DefPtr(ID3D11DeviceContext);
DefPtr(ID3D11Buffer);
DefPtr(ID3D11Query);
#undef DefPtr

namespace ms {

class GraphicsInterfaceD3D11 : public GraphicsInterface
{
public:
    GraphicsInterfaceD3D11(void *device);
    ~GraphicsInterfaceD3D11() override;
    void release() override;
    bool finish() override;
    bool writeBuffer(void *dst_buf, const void *src_mem, size_t write_size) override;

    ID3D11BufferPtr createStagingBuffer(size_t size);

private:
    ID3D11DevicePtr m_device;
    ID3D11DeviceContextPtr m_context;
    ID3D11QueryPtr m_query_event;
    std::vector<ID3D11BufferPtr> m_staging_buffers;
};


GraphicsInterface* CreateGraphicsInterfaceD3D11(void *device)
{
    if (!device)
        return nullptr;
    return new GraphicsInterfaceD3D11(device);
}

GraphicsInterfaceD3D11::GraphicsInterfaceD3D11(void *device)
    : m_device((ID3D11Device*)device)
{
    if (m_device != nullptr)
    {
        m_device->GetImmediateContext(&m_context);

        D3D11_QUERY_DESC qdesc = {D3D11_QUERY_EVENT , 0};
        m_device->CreateQuery(&qdesc, &m_query_event);
    }
}

GraphicsInterfaceD3D11::~GraphicsInterfaceD3D11()
{
}

void GraphicsInterfaceD3D11::release()
{
    delete this;
}

bool GraphicsInterfaceD3D11::finish()
{
    m_context->End(m_query_event);
    while (m_context->GetData(m_query_event, nullptr, 0, 0) == S_FALSE) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    m_staging_buffers.clear();
    return true;
}

bool GraphicsInterfaceD3D11::writeBuffer(void *dst_buf, const void *src_mem, size_t write_size)
{
    auto staging = createStagingBuffer(write_size);

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = m_context->Map(staging, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
        return false;

    memcpy(mapped.pData, src_mem, write_size);
    m_context->Unmap(staging, 0);
    m_context->CopyResource((ID3D11Buffer*)dst_buf, staging);
    m_staging_buffers.push_back(staging);
    return true;
}

ID3D11BufferPtr GraphicsInterfaceD3D11::createStagingBuffer(size_t size)
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)size;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11BufferPtr ret;
    m_device->CreateBuffer(&desc, nullptr, &ret);
    return ret;
}

} // namespace ms
#endif // msEnableD3D11
