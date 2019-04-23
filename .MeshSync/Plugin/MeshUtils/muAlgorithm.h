#pragma once

namespace mu {

// VERY SLOW. the number of input elements should be up to 10 or so.
template<class Iter>
Iter unique_unsorted(Iter begin, Iter end)
{
    for (auto i = begin; i + 1 < end; ++i) {
        for (;;) {
            auto p = std::find(i + 1, end, *i);
            if (p != end) {
                // shift elements
                auto q = p + 1;
                while (q != end)
                    *p++ = std::move(*q++);
                --end;
            }
            else
                break;
        }
    }
    return end;
}

} // namespace mu
