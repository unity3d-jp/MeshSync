
namespace ms {

template<class Entities>
static inline void SetupDataFlags(Entities& entities)
{
    for (auto& e : entities)
        e->setupDataFlags();
}

} //end namespace ms
