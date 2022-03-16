#pragma once

namespace ms {

union SceneCacheEncoderSettings
{
    struct {
        int compressionLevel;
    } zstd;
};


} // namespace ms
