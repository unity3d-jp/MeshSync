#pragma once
#include <string>
#include "muMisc.h"

namespace mu {

class ScopedTimer
{
public:
    ScopedTimer();
    float elapsed() const;

private:
    nanosec m_begin;
};

class ProfileTimer : public ScopedTimer
{
public:
    ProfileTimer(const char *mes, ...);
    ~ProfileTimer();

private:
    std::string m_message;
};

} // namespace mu

