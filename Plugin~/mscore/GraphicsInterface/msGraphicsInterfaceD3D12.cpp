#include "pch.h"
#include "msGraphicsInterface.h"

#ifdef msEnableD3D12
#define DefPtr(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))
DefPtr(ID3D12Device);
DefPtr(ID3D12CommandQueue);
DefPtr(ID3D12CommandAllocator);
DefPtr(ID3D12Fence);
DefPtr(ID3D12Resource);
#undef DefPtr


namespace ms {

class GraphicsInterfaceD3D12 : public GraphicsInterface
{
public:
    GraphicsInterfaceD3D12(void *device);
    ~GraphicsInterfaceD3D12();
    void release() override;
    bool finish() override;
    bool writeBuffer(void *dst_buf, const void *src_mem, size_t write_size) override;

    ID3D12ResourcePtr createStagingBuffer(size_t size);
    HRESULT copyResource(ID3D12Resource *dst, ID3D12Resource *src);

private:
    ID3D12DevicePtr m_device;
    ID3D12CommandQueuePtr m_cqueue;
    ID3D12CommandAllocatorPtr m_calloc;
    ID3D12FencePtr m_fence;
    uint64_t m_fence_value = 0;
    HANDLE m_fence_event;
};


GraphicsInterface* CreateGraphicsInterfaceD3D12(void *device)
{
    if (!device)
        return nullptr;
    return new GraphicsInterfaceD3D12(device);
}


GraphicsInterfaceD3D12::GraphicsInterfaceD3D12(void *device)
    : m_device((ID3D12Device*)device)
{
    // create command queue
    {
        D3D12_COMMAND_QUEUE_DESC desc;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_cqueue));
    }

    // create command allocator & list
    {
        m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_calloc));
    }

    // create signal
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    m_fence->Signal(1);

    m_fence_event = ::CreateEvent(nullptr, false, false, nullptr);
}

GraphicsInterfaceD3D12::~GraphicsInterfaceD3D12()
{
    if (m_fence_event)
        ::CloseHandle(m_fence_event);
}

void GraphicsInterfaceD3D12::release()
{
    delete this;
}


bool GraphicsInterfaceD3D12::finish()
{
    return false;
}

bool GraphicsInterfaceD3D12::writeBuffer(void *dst_buf, const void *src_mem, size_t write_size)
{
    auto *dst = (ID3D12Resource*)dst_buf;
    auto staging = createStagingBuffer(write_size);

    void *mapped;
    auto hr = staging->Map(0, nullptr, &mapped);
    if (FAILED(hr))
        return false;
    memcpy(mapped, src_mem, write_size);
    staging->Unmap(0, nullptr);

    return true;
}

ID3D12ResourcePtr GraphicsInterfaceD3D12::createStagingBuffer(size_t size)
{
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_GENERIC_READ;

    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap.CreationNodeMask = 0;
    heap.VisibleNodeMask = 0;
    heap.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12ResourcePtr ret;
    m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, state, nullptr, IID_PPV_ARGS(&ret));
    return ret;
}

} // namespace ms
#endif // msEnableD3D12
