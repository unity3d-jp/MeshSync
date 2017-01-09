#include "pch.h"
#include "giInternal.h"

#ifdef giSupportD3D9
#include <d3d9.h>

using Microsoft::WRL::ComPtr;

namespace gi {

class GraphicsInterfaceD3D9 : public GraphicsInterface
{
public:
    GraphicsInterfaceD3D9(void *device);
    ~GraphicsInterfaceD3D9() override;
    void release() override;

    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Result createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags) override;
    void   releaseTexture2D(void *tex) override;
    Result readTexture2D(void *dst, size_t read_size, void *src_tex, int width, int height, TextureFormat format) override;
    Result writeTexture2D(void *dst_tex, int width, int height, TextureFormat format, const void *src, size_t write_size) override;

    Result createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags) override;
    void   releaseBuffer(void *buf) override;
    Result mapBuffer(MapContext& ctx) override;
    Result unmapBuffer(MapContext& ctx) override;

private:
    ComPtr<IDirect3DSurface9> createStagingSurface(int width, int height, TextureFormat format);

private:
    ComPtr<IDirect3DDevice9> m_device;
    ComPtr<IDirect3DQuery9> m_query_event;
};


GraphicsInterface* CreateGraphicsInterfaceD3D9(void *device)
{
    if (!device) { return nullptr; }
    return new GraphicsInterfaceD3D9(device);
}


GraphicsInterfaceD3D9::GraphicsInterfaceD3D9(void *device)
    : m_device((IDirect3DDevice9*)device)
    , m_query_event(nullptr)
{
    if (m_device != nullptr)
    {
        m_device->CreateQuery(D3DQUERYTYPE_EVENT, &m_query_event);
    }
}

GraphicsInterfaceD3D9::~GraphicsInterfaceD3D9()
{
}

void GraphicsInterfaceD3D9::release()
{
    delete this;
}

void* GraphicsInterfaceD3D9::getDevicePtr() { return m_device.Get(); }
DeviceType GraphicsInterfaceD3D9::getDeviceType() { return DeviceType::D3D9; }



static D3DFORMAT GetInternalFormatD3D9(TextureFormat fmt)
{
    switch (fmt)
    {
    case TextureFormat::Ru8:     return D3DFMT_A8;
    case TextureFormat::RGu8:    return D3DFMT_A8L8;
    case TextureFormat::RGBAu8:  return D3DFMT_A8R8G8B8;

    case TextureFormat::Rf16:    return D3DFMT_R16F;
    case TextureFormat::RGf16:   return D3DFMT_G16R16F;
    case TextureFormat::RGBAf16: return D3DFMT_A16B16G16R16F;

    case TextureFormat::Ri16:    return D3DFMT_L16;
    case TextureFormat::RGi16:   return D3DFMT_G16R16;
    case TextureFormat::RGBAi16: return D3DFMT_A16B16G16R16;

    case TextureFormat::Rf32:    return D3DFMT_R32F;
    case TextureFormat::RGf32:   return D3DFMT_G32R32F;
    case TextureFormat::RGBAf32: return D3DFMT_A32B32G32R32F;
    }
    return D3DFMT_UNKNOWN;
}

ComPtr<IDirect3DSurface9> GraphicsInterfaceD3D9::createStagingSurface(int width, int height, TextureFormat format)
{
    D3DFORMAT internal_format = GetInternalFormatD3D9(format);
    if (internal_format == D3DFMT_UNKNOWN) { return nullptr; }

    auto ret = ComPtr<IDirect3DSurface9>();
    m_device->CreateOffscreenPlainSurface(width, height, internal_format, D3DPOOL_SYSTEMMEM, &ret, nullptr);
    return ret;
}


template<class T>
struct RGBA
{
    T r,g,b,a;
};

template<class T>
inline void BGRA_RGBA_conversion(RGBA<T> *data, int num_pixels)
{
    for (int i = 0; i < num_pixels; ++i) {
        std::swap(data[i].r, data[i].b);
    }
}

template<class T>
inline void copy_with_BGRA_RGBA_conversion(RGBA<T> *dst, int dst_pitch, const RGBA<T> *src, int src_pitch, int num_rows)
{
    int dst_width = dst_pitch / sizeof(RGBA<T>);
    int src_width = src_pitch / sizeof(RGBA<T>);
    int copy_width = std::min<int>(dst_width, src_width);

    for (int yi = 0; yi < num_rows; ++yi) {
        for (int xi = 0; xi < copy_width; ++xi) {
            RGBA<T>       &d = dst[dst_width * yi + xi];
            const RGBA<T> &s = src[src_width * yi + xi];
            d.r = s.b;
            d.g = s.g;
            d.b = s.r;
            d.a = s.a;
        }
    }
}

void GraphicsInterfaceD3D9::sync()
{
    m_query_event->Issue(D3DISSUE_END);
    auto hr = m_query_event->GetData(nullptr, 0, D3DGETDATA_FLUSH);
    if (hr != S_OK) {
        giLog("GetData() failed\n");
    }
}

Result GraphicsInterfaceD3D9::createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags /*flags*/)
{
    if (!dst_tex) { return Result::InvalidParameter; }

    IDirect3DTexture9* ret;
    auto fmt_d3d = GetInternalFormatD3D9(format);
    auto hr = m_device->CreateTexture((UINT)width, (UINT)height, 1, 0, fmt_d3d, D3DPOOL_MANAGED, &ret, nullptr);
    if (FAILED(hr)) { return TranslateReturnCode(hr); }

    *dst_tex = ret;

    if (data) {
        writeTexture2D(ret, width, height, format, data, width * height * GetTexelSize(format));
    }

    return Result::OK;
}

void GraphicsInterfaceD3D9::releaseTexture2D(void *tex)
{
    if (!tex) { return; }
    ((IUnknown*)tex)->Release();
}


static HRESULT LockSurfaceAndRead(void *dst, size_t read_size, IDirect3DSurface9 *surf, int width, int height, TextureFormat format)
{
    D3DLOCKED_RECT locked;
    auto hr = surf->LockRect(&locked, nullptr, D3DLOCK_READONLY);
    if (SUCCEEDED(hr))
    {
        auto *dst_pixels = (char*)dst;
        auto *src_pixels = (const char*)locked.pBits;
        int dst_pitch = width * GraphicsInterfaceD3D9::GetTexelSize(format);
        int src_pitch = locked.Pitch;
        int num_rows = std::min<int>(height, (int)ceildiv<size_t>(read_size, dst_pitch));
        CopyRegion(dst_pixels, dst_pitch, src_pixels, src_pitch, num_rows);

        surf->UnlockRect();

        // D3D9 の ARGB32 のピクセルの並びは BGRA になっているので並べ替える
        if (format == TextureFormat::RGBAu8) {
            BGRA_RGBA_conversion((RGBA<uint8_t>*)dst, int(read_size / 4));
        }
    }
    return hr;
};

Result GraphicsInterfaceD3D9::readTexture2D(void *dst, size_t read_size, void *src_tex, int width, int height, TextureFormat format)
{
    if (read_size == 0) { return Result::OK; }
    if (!dst || !src_tex) { return Result::InvalidParameter; }

    auto *tex = (IDirect3DTexture9*)src_tex;

    ComPtr<IDirect3DSurface9> surf_src;
    auto hr = tex->GetSurfaceLevel(0, &surf_src);
    if (FAILED(hr)) { return TranslateReturnCode(hr); }

    sync();

    // try direct access
    hr = LockSurfaceAndRead(dst, read_size, surf_src.Get(), width, height, format);
    if (SUCCEEDED(hr)) { return Result::OK; }


    // try copy-via-staging
    auto staging = createStagingSurface(width, height, format);
    if (staging == nullptr) { return Result::Unknown; }

    hr = m_device->GetRenderTargetData(surf_src.Get(), staging.Get());
    if (SUCCEEDED(hr)) {
        hr = LockSurfaceAndRead(dst, read_size, staging.Get(), width, height, format);
        if (SUCCEEDED(hr)) { return Result::OK; }
    }

    return TranslateReturnCode(hr);
}

static HRESULT LockSurfaceAndWrite(IDirect3DSurface9 *surf, int width, int height, TextureFormat format, const void *src, size_t write_size)
{
    D3DLOCKED_RECT locked;
    auto hr = surf->LockRect(&locked, nullptr, D3DLOCK_DISCARD);
    if (SUCCEEDED(hr))
    {
        auto *dst_pixels = (char*)locked.pBits;
        auto *src_pixels = (const char*)src;
        int dst_pitch = locked.Pitch;
        int src_pitch = GraphicsInterfaceD3D9::GetTexelSize(format) * width;
        int num_rows = std::min<int>(height, (int)ceildiv<size_t>(write_size, src_pitch));

        // こちらも ARGB32 の場合 BGRA に並べ替える必要がある
        if (format == TextureFormat::RGBAu8) {
            copy_with_BGRA_RGBA_conversion((RGBA<uint8_t>*)dst_pixels, dst_pitch, (RGBA<uint8_t>*)src_pixels, src_pitch, num_rows);
        }
        else {
            CopyRegion(dst_pixels, dst_pitch, src_pixels, src_pitch, num_rows);
        }
        surf->UnlockRect();
    }
    return hr;
};

Result GraphicsInterfaceD3D9::writeTexture2D(void *dst_tex, int width, int height, TextureFormat format, const void *src, size_t write_size)
{
    if (write_size == 0) { return Result::OK; }
    if (!dst_tex || !src) { return Result::InvalidParameter; }

    IDirect3DTexture9 *tex = (IDirect3DTexture9*)dst_tex;

    ComPtr<IDirect3DSurface9> surf_dst;
    auto hr = tex->GetSurfaceLevel(0, &surf_dst);
    if (FAILED(hr)) { return TranslateReturnCode(hr); }

    // try direct access
    hr = LockSurfaceAndWrite(surf_dst.Get(), width, height, format, src, write_size);
    if (SUCCEEDED(hr)) { return Result::OK; }


    // try copy-via-staging
    auto staging = createStagingSurface(width, height, format);
    if (staging == nullptr) { return Result::Unknown; }

    hr = LockSurfaceAndWrite(staging.Get(), width, height, format, src, write_size);
    if (SUCCEEDED(hr))
    {
        hr = m_device->UpdateSurface(staging.Get(), nullptr, surf_dst.Get(), nullptr);
        if (SUCCEEDED(hr)) { return Result::OK; }
    }

    return TranslateReturnCode(hr);
}


template<class BufferT>
static HRESULT MapBuffer(BufferT *buf, MapMode mode, void *& data)
{
    DWORD lock_mode = 0;
    switch (mode) {
    case MapMode::Read: lock_mode = D3DLOCK_READONLY; break;
    case MapMode::Write: lock_mode = D3DLOCK_DISCARD; break;
    }
    return buf->Lock(0, 0, &data, lock_mode);
}

static HRESULT MapBuffer(void *buf, BufferType type, MapMode mode, void*& data)
{
    switch (type) {
    case BufferType::Index: return MapBuffer((IDirect3DIndexBuffer9*)buf, mode, data);
    case BufferType::Vertex: return MapBuffer((IDirect3DVertexBuffer9*)buf, mode, data);
    default: return E_INVALIDARG;
    }
}

static HRESULT UnmapBuffer(void *buf, BufferType type)
{
    switch (type) {
    case BufferType::Index: return ((IDirect3DIndexBuffer9*)buf)->Unlock();
    case BufferType::Vertex: return ((IDirect3DVertexBuffer9*)buf)->Unlock();
    default: return E_INVALIDARG;
    }
}

Result GraphicsInterfaceD3D9::createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags /*flags*/)
{
    if (!dst_buf) { return Result::InvalidParameter; }

    *dst_buf = nullptr;
    if (type == BufferType::Index) {
        IDirect3DIndexBuffer9 *ret;
        auto hr = m_device->CreateIndexBuffer(UINT(size), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &ret, nullptr);
        if (FAILED(hr)) { return TranslateReturnCode(hr); }
        *dst_buf = ret;
    }
    else if (type == BufferType::Vertex) {
        IDirect3DVertexBuffer9 *ret;
        auto hr = m_device->CreateVertexBuffer((UINT)size, 0, 0, D3DPOOL_MANAGED, &ret, nullptr);
        if (FAILED(hr)) { return TranslateReturnCode(hr); }
        *dst_buf = ret;
    }

    if (!*dst_buf) {
        return Result::Unknown;
    }

    if (data) {
        writeBuffer(*dst_buf, data, size, type);
    }

    return Result::OK;
}

void GraphicsInterfaceD3D9::releaseBuffer(void *buf)
{
    if (!buf) { return; }
    ((IUnknown*)buf)->Release();
}

Result GraphicsInterfaceD3D9::mapBuffer(MapContext& ctx)
{
    if (!ctx.resource) { return Result::InvalidParameter; }
    auto hr = MapBuffer(ctx.resource, ctx.type, ctx.mode, ctx.data_ptr);
    return TranslateReturnCode(hr);
}

Result GraphicsInterfaceD3D9::unmapBuffer(MapContext& ctx)
{
    if (!ctx.resource) { return Result::InvalidParameter; }
    auto hr = UnmapBuffer(ctx.resource, ctx.type);
    ctx.data_ptr = nullptr;
    return TranslateReturnCode(hr);
}

} // namespace gi
#endif // giSupportD3D9
