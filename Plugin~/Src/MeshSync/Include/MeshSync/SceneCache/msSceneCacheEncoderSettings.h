#pragma once

namespace ms {

union SceneCacheEncoderSettings
{
    struct {
        int compression_level;
    } zstd;
};


} // namespace ms
