#include "pch.h"
#include "Test.h"
#include "MeshSync/Utility/msNetworkUtility.h" //ms::NetworkUtils::IsInLocalNetwork

using namespace mu;

TestCase(Test_CheckLocalNetworkAccess)
{
    const std::string allowedHostPorts[] = {
        "127.0.0.1:8080",
        "localhost:8080",
        "192.168.1.30:8080",
    };

    const uint32_t numAllowedHostPorts = sizeof(allowedHostPorts) / sizeof(allowedHostPorts[0]);
    for (uint32_t i=0;i<numAllowedHostPorts;++i) {

        const bool inLocalNetwork = ms::NetworkUtils::IsInLocalNetwork(allowedHostPorts[i]);
        Expect(inLocalNetwork);
    }
}


TestCase(Test_CheckPublicNetworkAccess)
{
    const std::string publicHostPorts[] = {
        "127.0.0.1g.jp:8080",
        "s-11.22.33.44-555.0.0.1-12341234-aa-b.foo.bar.zoo:8080",
    };

    const uint32_t numPublicHostPorts= sizeof(publicHostPorts) / sizeof(publicHostPorts[0]);
    for (uint32_t i=0;i<numPublicHostPorts;++i) {

        const bool inLocalNetwork = ms::NetworkUtils::IsInLocalNetwork(publicHostPorts[i]);
        Expect(!inLocalNetwork);
    }
}


