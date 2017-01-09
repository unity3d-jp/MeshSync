#include "pch.h"
#include "giInternal.h"

namespace gi {

#ifdef _WIN32
DXGI_FORMAT GetDXGIFormat(TextureFormat fmt)
{
    switch (fmt)
    {
    case TextureFormat::Ru8:     return DXGI_FORMAT_R8_UNORM;
    case TextureFormat::RGu8:    return DXGI_FORMAT_R8G8_UNORM;
    case TextureFormat::RGBAu8:  return DXGI_FORMAT_R8G8B8A8_UNORM;

    case TextureFormat::Rf16:    return DXGI_FORMAT_R16_FLOAT;
    case TextureFormat::RGf16:   return DXGI_FORMAT_R16G16_FLOAT;
    case TextureFormat::RGBAf16: return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case TextureFormat::Ri16:    return DXGI_FORMAT_R16_SNORM;
    case TextureFormat::RGi16:   return DXGI_FORMAT_R16G16_SNORM;
    case TextureFormat::RGBAi16: return DXGI_FORMAT_R16G16B16A16_SNORM;

    case TextureFormat::Rf32:    return DXGI_FORMAT_R32_FLOAT;
    case TextureFormat::RGf32:   return DXGI_FORMAT_R32G32_FLOAT;
    case TextureFormat::RGBAf32: return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case TextureFormat::Ri32:    return DXGI_FORMAT_R32_SINT;
    case TextureFormat::RGi32:   return DXGI_FORMAT_R32G32_SINT;
    case TextureFormat::RGBAi32: return DXGI_FORMAT_R32G32B32A32_SINT;
    }
    return DXGI_FORMAT_UNKNOWN;
}

Result TranslateReturnCode(HRESULT hr)
{
    switch (hr) {
    case S_OK: return Result::OK;
    case E_OUTOFMEMORY: return Result::OutOfMemory;
    case E_INVALIDARG: return Result::InvalidParameter;
    }
    return Result::Unknown;
}
#endif

} // namespace gi
