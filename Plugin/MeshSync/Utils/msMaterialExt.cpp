#include "pch.h"
#include "msMaterialExt.h"

namespace ms {

inline TexturePtr MakeTmpTexture(int id)
{
    auto ret = Texture::create();
    ret->id = id;
    return ret;
}

void StandardMaterial::setColor(float4 v)
{
    addParam({ "_BaseColor", v });
}
float4 StandardMaterial::getColor() const
{
    auto *p = findParam("_BaseColor");
    return p ? p->getVector() : float4::zero();
}

void StandardMaterial::setEmission(float4 v)
{
    addParam({ "_Emission", v });
}
float4 StandardMaterial::getEmission() const
{
    auto *p = findParam("_Emission");
    return p ? p->getVector() : float4::zero();
}

void StandardMaterial::setMetallic(float v)
{
    addParam({ "_Metallic", v });
}
float StandardMaterial::getMetallic() const
{
    auto *p = findParam("_Metallic");
    return p ? p->getFloat() : 0.0f;
}

void StandardMaterial::setSmoothness(float v)
{
    addParam({ "_Glossiness", v });
}
float StandardMaterial::getSmoothness() const
{
    auto *p = findParam("_Glossiness");
    return p ? p->getFloat() : 0.0f;
}

void StandardMaterial::setColorMap(int v)
{
    setColorMap(MakeTmpTexture(v));
}
void StandardMaterial::setColorMap(TexturePtr v)
{
    if (v && v->id != InvalidID)
        addParam({ "_MainTex", v });
    else
        eraseParam("_MainTex");
}
int StandardMaterial::getColorMap() const
{
    auto *p = findParam("_MainTex");
    return p ? p->getTexture() : InvalidID;
}

void StandardMaterial::setEmissionMap(int v)
{
    setEmissionMap(MakeTmpTexture(v));
}
void StandardMaterial::setEmissionMap(TexturePtr v)
{
    if (v && v->id != InvalidID)
        addParam({ "_EmissionMap", v });
    else
        eraseParam("_EmissionMap");
}
int StandardMaterial::getEmissionMap() const
{
    auto *p = findParam("_EmissionMap");
    return p ? p->getTexture() : InvalidID;
}

void StandardMaterial::setMetallicMap(int v)
{
    setMetallicMap(MakeTmpTexture(v));
}
void StandardMaterial::setMetallicMap(TexturePtr v)
{
    if (v && v->id != InvalidID)
        addParam({ "_MetallicGlossMap", v });
    else
        eraseParam("_MetallicGlossMap");
}
int StandardMaterial::getMetallicMap() const
{
    auto *p = findParam("_MetallicGlossMap");
    return p ? p->getTexture() : InvalidID;
}

void StandardMaterial::setNormalMap(int v)
{
    setNormalMap(MakeTmpTexture(v));
}
void StandardMaterial::setNormalMap(TexturePtr v)
{
    if (v && v->id != InvalidID)
        addParam({ "_BumpMap", v });
    else
        eraseParam("_BumpMap");
}
int StandardMaterial::getNormalMap() const
{
    auto *p = findParam("_BumpMap");
    return p ? p->getTexture() : InvalidID;
}

} // namespace ms
