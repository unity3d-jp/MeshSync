#include "pch.h"
#include "msmbUtils.h"

bool IsNull(FBModel * src)
{
    return src && src->Is(FBModelNull::TypeInfo);
}

bool IsCamera(FBModel *src)
{
    return src && src->Is(FBCamera::TypeInfo);
}

bool IsLight(FBModel *src)
{
    return src && src->Is(FBLight::TypeInfo);
}

bool IsBone(FBModel *src)
{
    return src &&
        (src->Is(FBModelSkeleton::TypeInfo) || IsNull(src));
}

bool IsMesh(FBModel* src)
{
    return src && src->ModelVertexData;
}

const char* GetName(FBModel *src)
{
    return src->LongName;
}

std::string GetPath(FBModel *src)
{
    std::string ret;
    if (src->Parent)
        ret = GetPath(src->Parent);
    ret += '/';
    ret += GetName(src);
    return ret;
}

std::tuple<double, double> GetTimeRange(FBTake *take)
{
    FBTimeSpan timespan = take->LocalTimeSpan;
    return { timespan.GetStart().GetSecondDouble(), timespan.GetStop().GetSecondDouble() };
}

static void EnumerateAllNodesImpl(FBModel *node, const std::function<void(FBModel*)>& body)
{
    body(node);

    int num_children = node->Children.GetCount();
    for (int i = 0; i < num_children; i++)
        EnumerateAllNodesImpl(node->Children[i], body);
}
void EnumerateAllNodes(const std::function<void(FBModel*)>& body)
{
    auto& scene = FBSystem::TheOne().Scene;
    Each(scene->Cameras, [&body](FBCamera *cam) {
        if (cam->LongName == "Producer Perspective")
            body(cam);
    });
    EnumerateAllNodesImpl(scene->RootModel, body);
}


#ifdef mscDebug

void DbgPrintProperties(FBPropertyManager& properties)
{
    Each(properties, [](FBProperty *p) {
        const char *name = p->GetName();
        const char *value = p->AsString();
        const char *tname = p->GetPropertyTypeName();
        mu::Print(" %s : %s (%s)\n", name, value, tname);
    });
}

static void DbgPrintAnimationNodeRecursive(FBAnimationNode * node, int depth = 0)
{
    if (!node)
        return;

    char indent[256];
    int i;
    for (i = 0; i < depth; ++i)
        indent[i] = ' ';
    indent[i] = '\0';

    const char *folder = nullptr;
    const char *ns = nullptr;
    {
        FBFolder *fbfolder = node->Folder;
        if (fbfolder)
            folder = fbfolder->Name.AsString();
    }
    {
        auto *fbns = node->GetOwnerNamespace();
        if (fbns)
            ns = fbns->Name.AsString();
    }
    const char *name = node->Name.AsString();
    const char *fullname = node->GetFullName();
    const char *classname = node->ClassName();

    double value;
    node->ReadData(&value);
    mu::Print("%sAnimationNode %s : %lf (classname:%s fullname:%s namespace:%s folder:%s)\n",
        indent, name, value, classname, fullname, ns, folder);

    Each(node->Nodes, [depth](FBAnimationNode *n) {
        DbgPrintAnimationNodeRecursive(n, depth + 1);
    });
}

void DbgPrintAnimationNode(FBAnimationNode * node)
{
    if (!node)
        return;
    while (node->Parent)
        node = node->Parent;
    DbgPrintAnimationNodeRecursive(node);
}

void DbgPrintCluster(FBModel *model)
{
    FBCluster *cluster = model->Cluster;
    if (!cluster)
        return;

    FBModelVertexData *vd = model->ModelVertexData;
    RawVector<float> total_weights;
    total_weights.resize_zeroclear(vd->GetVertexCount());

    mu::Print("Cluster %s:\n", cluster->LongName.AsString());
    int num_links = cluster->LinkGetCount();
    for (int li = 0; li < num_links; ++li) {
        ClusterScope scope(cluster, li);

        auto bone = cluster->LinkGetModel(li);
        mu::Print("  Bone %s:\n", bone->LongName.AsString());

        // weights
        int n = cluster->VertexGetCount();
        mu::Print("    %d vertices:\n", n);
        for (int vi = 0; vi < n; ++vi) {
            int i = cluster->VertexGetNumber(vi);
            float w = (float)cluster->VertexGetWeight(vi);
            mu::Print("      %d - %f:\n", i, w);
            total_weights[i] += w;
        }
    }

    mu::Print("  Weights %s:\n", cluster->LongName.AsString());
    for (float v : total_weights) {
        mu::Print("    %f\n", v);
    }
}

#endif
