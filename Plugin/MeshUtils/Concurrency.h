#pragma once

// available options:
//   muEnablePPL
//   muEnableTBB
//   muEnableISPC
//   muEnableAMP

#ifdef _WIN32
    #define muEnablePPL
    #define muEnableISPC
    #define muEnableAMP
#endif


#if defined(muEnablePPL)
    #include <ppl.h>
#elif defined(muEnableTBB)
    #include <tbb/tbb.h>
#endif

namespace mu {

template<class Index, class Body>
inline void parallel_for(Index begin, Index end, const Body& body)
{
#if defined(muEnablePPL)
    concurrency::parallel_for(begin, end, body);
#elif defined(muEnableTBB)
    tbb::parallel_for(begin, end, body);
#else
    for (; begin != end; ++begin) { body(begin); }
#endif
}

template<class Iter, class Body>
inline void parallel_for_each(Iter begin, Iter end, const Body& body)
{
#if defined(muEnablePPL)
    concurrency::parallel_for_each(begin, end, body);
#elif defined(muEnableTBB)
    tbb::parallel_for_each(begin, end, body);
#else
    for (; begin != end; ++begin) { body(*begin); }
#endif
}

template<class Body>
auto invoke(const Body& body) -> decltype(body()) { return body(); }

template <class... Bodies>
inline void parallel_invoke(Bodies... bodies)
{
#if defined(muEnablePPL)
    concurrency::parallel_invoke(bodies...);
#elif defined(muEnableTBB)
    tbb::parallel_invoke(bodies...);
#else
    invoke(std::forward<Bodies>(bodies)...);
#endif
}
} // namespace ms

