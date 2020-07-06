#pragma once
#include <string>

namespace ms {

class NetworkUtils
{
public:
    static bool IsLocalHost(const std::string& hostAndPort);
};


} // namespace ms
