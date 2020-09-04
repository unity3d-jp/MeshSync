#include "pch.h"
#include "MeshSync/Utility/msNetworkUtility.h"

#include <Poco/RegularExpression.h>


namespace ms {

bool NetworkUtils::IsInLocalNetwork(const std::string& hostAndPort) {
    const uint32_t MAX_TOKENS = 3;
    std::vector<std::string> tokens(MAX_TOKENS);
    std::istringstream input(hostAndPort);

    uint32_t i = 0;
    while (i < MAX_TOKENS && getline(input, tokens[i], ':')) {
        ++i;
    }

    if (i >= MAX_TOKENS)
        return false;


    static const Poco::RegularExpression regex(
        "(^localhost$)"
        "|(^127\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$)"
        "|(^10\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$)"
        "|(^172\\.1[6-9]{1}[0-9]{0,1}\\.[0-9]{1,3}\\.[0-9]{1,3}$)"
        "|(^172\\.2[0-9]{1}[0-9]{0,1}\\.[0-9]{1,3}\\.[0-9]{1,3}$)"
        "|(^172\\.3[0-1]{1}[0-9]{0,1}\\.[0-9]{1,3}\\.[0-9]{1,3}$)|(^192\\.168\\.[0-9]{1,3}\\.[0-9]{1,3}$)");


    return regex.match(tokens[0]);

}

} // namespace ms
