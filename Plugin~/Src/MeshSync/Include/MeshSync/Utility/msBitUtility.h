#pragma once

namespace ms {

class BitUtility{
public:
    inline static void Set(uint32_t* val, uint32_t bitPos, const bool bitValue);
    inline static bool Get(const uint32_t* val, uint32_t bitPos);
};

void BitUtility::Set(uint32_t* val, uint32_t bitPos, const bool bitValue) {
    assert(bitPos < (sizeof(bitPos) * 8) && "BitUtility::Set() invalid index");
    const uint32_t mask = (1 << bitPos);
    if (bitValue) {
        *val |= mask;
    } else {
        *val &= ~mask;    
    }
}

bool BitUtility::Get(const uint32_t* val, uint32_t bitPos) {
    assert(bitPos < (sizeof(bitPos) * 8) && "BitUtility::Get() invalid index");
    return (*val & (1 << bitPos));
}

} // namespace ms
