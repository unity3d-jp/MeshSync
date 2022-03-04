#pragma once

namespace ms {

class EncodingUtility {
public:
    static std::tuple<int, int> GetZSTDCompressionLevelRange();
    static int ClampZSTDCompressionLevel(int v);
    static int GetZSTDDefaultCompressionLevel();
};

} //end namespace ms
