#pragma once
#include <string>

namespace ms {

class NetworkUtils
{
public:
    static bool IsInLocalNetwork(const std::string& hostAndPort);
};


} // namespace ms
