#pragma once
#include <random>

namespace ms {

class IdUtility {
private: 
    int m_message_count = 0;
    int session_id = -1;

    public:
        inline static int GenerateSessionId();
        inline int GetNextMessageId();
        inline int GetSessionId();
    };

    int IdUtility::GenerateSessionId() {
        std::uniform_int_distribution<> d(0, 0x70000000);
        std::mt19937 r;
        r.seed(std::random_device()());
        return d(r);
    }

    int IdUtility::GetNextMessageId() {
        return m_message_count++;
    }

    int IdUtility::GetSessionId() {
        if (session_id == -1) {
            session_id = GenerateSessionId();
        }
        return session_id;
    }

} // namespace ms

