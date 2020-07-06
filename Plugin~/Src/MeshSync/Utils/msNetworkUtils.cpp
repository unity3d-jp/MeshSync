#include "pch.h"
#include "msNetworkUtils.h"

#include "msMisc.h" //StartsWith()

namespace ms {

bool NetworkUtils::IsLocalHost(const std::string& hostAndPort) {
    return (StartsWith(hostAndPort,"localhost") || StartsWith(hostAndPort,"127.0.0.1"));
}

} // namespace ms
