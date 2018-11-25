#include "pch.h"
#include "muHalf.h"

namespace mu {
    
const float snorm8::C = 127.0f;
const float snorm8::R = 1.0f / 127.0f;

const float unorm8::C = 255.0f;
const float unorm8::R = 1.0f / 255.0f;

const float unorm8n::C = 255.0f;
const float unorm8n::R = 1.0f / 255.0f;

const float snorm16::C = 32767.0f;
const float snorm16::R = 1.0f / 32767.0f;

const float unorm16::C = 65535.0f;
const float unorm16::R = 1.0f / 65535.0f;

const double snorm24::C = 2147483647.0;
const double snorm24::R = 1.0 / 2147483647.0;

const double snorm32::C = 2147483647.0;
const double snorm32::R = 1.0 / 2147483647.0;

} // namespace mu
