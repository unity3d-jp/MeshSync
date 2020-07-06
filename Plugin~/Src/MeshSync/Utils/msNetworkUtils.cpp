#include "pch.h"
#include "msNetworkUtils.h"

#include "msMisc.h" //StartsWith()

namespace ms {

bool NetworkUtils::IsLocalHost(const std::string& hostAndPort) {
    const uint32_t MAX_TOKENS = 3;
    std::vector<std::string> tokens(MAX_TOKENS);
    std::istringstream input(hostAndPort);

    uint32_t i = 0;
    while (i < MAX_TOKENS && getline(input, tokens[i], ':')) {
        ++i;
    }


    return (i < MAX_TOKENS && (tokens[0] == "localhost" || tokens[0] == "127.0.0.1"));
}

} // namespace ms
