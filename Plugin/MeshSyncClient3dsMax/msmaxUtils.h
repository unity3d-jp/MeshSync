#pragma once

#include "MeshSync/MeshSync.h"

std::wstring GetNameW(INode *n);
std::string  GetName(INode *n);
std::wstring GetPathW(INode *n);
std::string  GetPath(INode *n);


// Body: [](INode *node) -> void
template<class Body>
inline void EachNode(NodeEventNamespace::NodeKeyTab& nkt, const Body& body)
{
    int count = nkt.Count();
    for (int i = 0; i < count; ++i) {
        if (auto *n = NodeEventNamespace::GetNodeByKey(nkt[i])) {
            body(n);
        }
    }
}


namespace detail {

    template<class Body>
    class EnumerateAllNodeImpl : public ITreeEnumProc
    {
    public:
        const Body & body;
        int ret;

        EnumerateAllNodeImpl(const Body& b, bool ignore_childrern = false)
            : body(b)
            , ret(ignore_childrern ? TREE_IGNORECHILDREN : TREE_CONTINUE)
        {}

        int callback(INode *node) override
        {
            body(node);
            return ret;
        }
    };

} // namespace detail

// Body: [](INode *node) -> void
template<class Body>
inline void EnumerateAllNode(const Body& body)
{
    if (auto *scene = GetCOREInterface7()->GetScene()) {
        detail::EnumerateAllNodeImpl<Body> cb(body);
        scene->EnumTree(&cb);
    }
    else {
        mscTrace("EnumerateAllNode() failed!!!\n");
    }
}

inline void DumpNodes(NodeEventNamespace::NodeKeyTab& nkt)
{
    EachNode(nkt, [](INode *node) {
        mscTraceW(L"node: %s\n", node->GetName());
    });
}
