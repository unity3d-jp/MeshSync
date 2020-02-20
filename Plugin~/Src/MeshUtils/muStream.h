#pragma once
#include <iostream>
#include "muRawVector.h"

namespace mu {

class MemoryStreamBuf : public std::streambuf
{
friend class MemoryStream;
public:
    static const size_t default_bufsize = 1024 * 128;

    MemoryStreamBuf();
    MemoryStreamBuf(RawVector<char>&& buf);
    void reset();
    void resize(size_t n);
    void swap(RawVector<char>& buf);

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode mode) override;
    pos_type seekpos(pos_type pos, std::ios_base::openmode mode) override;
    int overflow(int c) override;
    int underflow() override;
    int sync() override;

    RawVector<char> buffer;
    uint64_t wcount = 0;
    uint64_t rcount = 0;
};

class MemoryStream : public std::iostream
{
public:
    MemoryStream();
    MemoryStream(RawVector<char>&& buf);
    void reset();
    void resize(size_t n);
    void swap(RawVector<char>& buf);

    const RawVector<char>& getBuffer() const;
    RawVector<char>&& moveBuffer();
    uint64_t getWCount() const;
    uint64_t getRCount() const;

    char* gskip(size_t n); // return current read pointer and advance n byte

private:
    MemoryStreamBuf m_buf;
};


class CounterStreamBuf : public std::streambuf
{
public:
    static const size_t default_bufsize = 1024 * 4;

    CounterStreamBuf();
    int overflow(int c) override;
    int sync() override;
    void reset();

    uint64_t m_size = 0;
};

class CounterStream : public std::ostream
{
public:
    CounterStream();
    uint64_t size() const;
    void reset();

private:
    CounterStreamBuf m_buf;
};

} // namespace mu
