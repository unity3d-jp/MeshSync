#include "pch.h"
#include "msFoundation.h"
#include "msSceneGraphImpl.h"

namespace ms {

MemoryStreamBuf::MemoryStreamBuf()
{
    resize(default_bufsize);
}

void MemoryStreamBuf::reset()
{
    this->setp(buffer.data(), buffer.data(), buffer.data() + buffer.size());
    this->setg(buffer.data(), buffer.data(), buffer.data() + buffer.size());
}

void MemoryStreamBuf::resize(size_t n)
{
    buffer.resize(n);
    reset();
}

void MemoryStreamBuf::swap(RawVector<char>& buf)
{
    buffer.swap(buf);
    reset();
}

MemoryStreamBuf::int_type MemoryStreamBuf::overflow(int_type c)
{
    ++wcount;
    return c;
}

MemoryStreamBuf::int_type MemoryStreamBuf::underflow()
{
    ++rcount;
    return 0;
}

int MemoryStreamBuf::sync()
{
    wcount += uint64_t(this->pptr() - this->pbase());
    rcount += uint64_t(this->gptr() - this->eback());
    return 0;
}

MemoryStream::MemoryStream() : std::iostream(&m_buf) {}
void MemoryStream::reset() { m_buf.reset(); }
void MemoryStream::resize(size_t n) { m_buf.resize(n); }
void MemoryStream::swap(RawVector<char>& buf) { m_buf.swap(buf); }
const RawVector<char>& MemoryStream::getBuffer() const { return m_buf.buffer; }
uint64_t MemoryStream::getWCount() const { return m_buf.wcount; }
uint64_t MemoryStream::getRCount() const { return m_buf.rcount; }


// Identifier
Identifier::Identifier() {}
Identifier::Identifier(const std::string& p, int i) : name(p), id(i) {}

void Identifier::serialize(std::ostream& os) const
{
    write(os, name); write(os, id);
}
void Identifier::deserialize(std::istream& is)
{
    read(is, name); read(is, id);
}

} // namespace ms
