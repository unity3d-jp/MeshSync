#pragma once
#include <string>
#include "muMisc.h"

namespace mu {

class ScopedTimer
{
public:
    ScopedTimer(const char *mes, ...);
    ~ScopedTimer();

private:
    std::string m_message;
    nanosec m_begin;
};

} // namespace mu

