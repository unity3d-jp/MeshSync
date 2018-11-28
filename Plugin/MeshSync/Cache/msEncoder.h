#pragma once

namespace ms {

class Encoder
{
public:
    virtual ~Encoder();
    virtual void encode(RawVector<char>& dst, const RawVector<char>& src) = 0;
    virtual void decode(RawVector<char>& dst, const RawVector<char>& src) = 0;
};
using EncoderPtr = std::shared_ptr<Encoder>;

EncoderPtr CreatePlainEncoder();
EncoderPtr CreateZSTDEncoder();

} // namespace ms
