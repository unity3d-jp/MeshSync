#pragma once

#ifdef _WIN32
#define msEnableD3D11
#define msEnableD3D12
#endif // _WIN32

namespace ms {

enum class DeviceType
{
    Unknown,
    D3D11,
    D3D12,
    OpenGL,
    Vulkan,
    Metal,
};

class GraphicsInterface
{
protected:
    friend void ReleaseGraphicsInterface();
    virtual ~GraphicsInterface();
    virtual void release() = 0;

public:
    virtual bool finish() = 0;
    virtual bool writeBuffer(void *dst_buf, const void *src_mem, size_t write_size) = 0;
};


// create GraphicsInterface instance and store it internally (that can get by GetGraphicsInterface()).
// if instance already exists, delete old one and re-create.
GraphicsInterface* CreateGraphicsInterface(DeviceType type, void *device_ptr);

// return instance created by CreateGraphicsInterface()
GraphicsInterface* GetGraphicsInterface();

// release existing instance
void ReleaseGraphicsInterface();

} // namespace ms
