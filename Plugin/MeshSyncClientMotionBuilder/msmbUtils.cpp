#include "pch.h"
#include "msmbUtils.h"

bool IsCamera(FBModel *src)
{
    return src->Is(FBCamera::TypeInfo);
}

bool IsLight(FBModel *src)
{
    return src->Is(FBLight::TypeInfo);
}

bool IsBone(FBModel *src)
{
    return src->Is(FBModelSkeleton::TypeInfo);
}

bool IsMesh(FBModel* src)
{
    return src->ModelVertexData;
}

std::string GetFilename(const char *src)
{
    int last_separator = 0;
    for (int i = 0; src[i] != '\0'; ++i) {
        if (src[i] == '\\' || src[i] == '/')
            last_separator = i + 1;
    }
    return std::string(src + last_separator);
}

std::string GetFilenameWithoutExtension(const char * src)
{
    int last_separator = 0;
    int last_comma = 0;
    for (int i = 0; src[i] != '\0'; ++i) {
        if (src[i] == '\\' || src[i] == '/')
            last_separator = i + 1;
        if (src[i] == '.')
            last_comma = i;
    }

    if (last_comma > last_separator)
        return std::string(src + last_separator, src + last_comma);
    else
        return std::string(src + last_separator);
}

std::string GetPath(FBModel *src)
{
    std::string ret;
    if (src->Parent)
        ret = GetPath(src->Parent);
    ret += '/';
    ret += src->LongName;
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
    EnumerateAllNodesImpl(FBSystem::TheOne().RootModel, body);
}


ms::float4x4 to_float4x4(const FBMatrix& src)
{
    auto m = (const double*)&src;
    return { {
         (float)m[0], (float)m[1], (float)m[2], (float)m[3],
         (float)m[4], (float)m[5], (float)m[6], (float)m[7],
         (float)m[8], (float)m[9], (float)m[10], (float)m[11],
         (float)m[12], (float)m[13], (float)m[14], (float)m[15],
    } };
}


void ABGR2RGBA(char *dst, const char *src, int num)
{
    for (int pi = 0; pi < num; ++pi) {
        dst[0] = src[3];
        dst[1] = src[2];
        dst[2] = src[1];
        dst[3] = src[0];
        dst += 4;
        src += 4;
    }
}

void ARGB2RGBA(char *dst, const char *src, int num)
{
    for (int pi = 0; pi < num; ++pi) {
        dst[0] = src[1];
        dst[1] = src[2];
        dst[2] = src[3];
        dst[3] = src[0];
        dst += 4;
        src += 4;
    }
}

void BGRA2RGBA(char *dst, const char *src, int num)
{
    for (int pi = 0; pi < num; ++pi) {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
        dst[3] = src[3];
        dst += 4;
        src += 4;
    }
}

void BGR2RGB(char *dst, const char *src, int num)
{
    for (int pi = 0; pi < num; ++pi) {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
        dst += 3;
        src += 3;
    }
}
