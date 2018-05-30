#pragma once

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

inline void DumpNodes(NodeEventNamespace::NodeKeyTab& nkt)
{
    EachNode(nkt, [](INode *node) {
        mscTraceW(L"node: %s\n", node->GetName());
    });
}
