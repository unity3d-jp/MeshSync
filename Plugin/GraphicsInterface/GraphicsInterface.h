#pragma once

namespace gi {

enum class DeviceType
{
    Unknown,
    D3D9,
    D3D11,
    D3D12,
    OpenGL,
    Vulkan,
    PS4,
};

enum class Result
{
    OK,
    Unknown,
    NotAvailable,
    InvalidParameter,
    InvalidOperation,
    OutOfMemory,
};

enum class TextureFormat
{
    Unknown = 0,

    Ru8,
    RGu8,
    RGBAu8,
    Rf16,
    RGf16,
    RGBAf16,
    Ri16,
    RGi16,
    RGBAi16,
    Rf32,
    RGf32,
    RGBAf32,
    Ri32,
    RGi32,
    RGBAi32,
};

enum class BufferType
{
    Unknown,
    Index,
    Vertex,
    Constant,
    Compute,
    End,
};

enum class ResourceFlags
{
    None            = 0x0,
    CPU_Write       = 0x1,
    CPU_Read        = 0x2,
    CPU_ReadWrite   = CPU_Read | CPU_Write,
};
inline ResourceFlags operator|(ResourceFlags a, ResourceFlags b) { return ResourceFlags((int)a | (int)b); }

enum class MapMode
{
    Unknown,
    Read,
    Write,
    //ReadWrite,
    //WriteDiscard,
};

struct MapContext
{
    void *data_ptr = nullptr;
    void *resource = nullptr;
    void *staging_resource = nullptr;
    BufferType type = BufferType::Unknown;
    MapMode mode = MapMode::Unknown;
    unsigned int size = 0;
    bool keep_staging_resource = false;
};

class GraphicsInterface
{
protected:
    friend void ReleaseGraphicsInterface();
    virtual ~GraphicsInterface();
    virtual void release() = 0;

public:
    virtual void* getDevicePtr() = 0;
    virtual DeviceType getDeviceType() = 0;

    virtual void    sync() = 0;

    virtual Result  createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags = ResourceFlags::None) = 0;
    virtual void    releaseTexture2D(void *tex) = 0;
    virtual Result  readTexture2D(void *dst, size_t read_size, void *src_tex, int width, int height, TextureFormat format) = 0;
    virtual Result  writeTexture2D(void *dst_tex, int width, int height, TextureFormat format, const void *src, size_t write_size) = 0;

    virtual Result  createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags = ResourceFlags::None) = 0;
    virtual void    releaseBuffer(void *buf) = 0;
    virtual Result  mapBuffer(MapContext& ctx) = 0;
    virtual Result  unmapBuffer(MapContext& ctx) = 0;
    void releaseStagingResource(MapContext& ctx);
    Result readBuffer(void *dst_mem, void *src_buf, size_t read_size, BufferType type);
    Result writeBuffer(void *dst_buf, const void *src_mem, size_t write_size, BufferType type);

    static int GetTexelSize(TextureFormat format);
};


// create GraphicsInterface instance and store it internally (that can get by GetGraphicsInterface()).
// if instance already exists, delete old one and re-create.
// device_ptr:
//  IDirect3DDevice9* on D3D9
//  ID3D11Device* on D3D11
//  ID3D12Device* on D3D12
//  nullptr on OpenGL
//  {VkPhysicalDevice, VkDevice} on Vulkan
//    e.g:
//      void *devices[] = {physical_device, device};
//      CreateGraphicsInterface(DeviceType::Vulkan, devices);
GraphicsInterface* CreateGraphicsInterface(DeviceType type, void *device_ptr);

// return instance created by CreateGraphicsInterface()
GraphicsInterface* GetGraphicsInterface();

// release existing instance
void ReleaseGraphicsInterface();

} // namespace gi
