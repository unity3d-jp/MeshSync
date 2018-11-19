#include "pch.h"
#include "msMaterialExt.h"

namespace ms {

// shader keywords
static const char _Color[] = "_Color";
static const char _EmissionColor[] = "_EmissionColor";
static const char _Metallic[] = "_Metallic";
static const char _Glossiness[] = "_Glossiness";
static const char _MainTex[] = "_MainTex";
static const char _EmissionMap[] = "_EmissionMap";
static const char _MetallicGlossMap[] = "_MetallicGlossMap";
static const char _BumpScale[] = "_BumpScale";
static const char _BumpMap[] = "_BumpMap";

inline TexturePtr MakeTmpTexture(int id)
{
    auto ret = Texture::create();
    ret->id = id;
    return ret;
}

void StandardMaterial::setColor(float4 v)
{
    addParam({ _Color, v });
}
float4 StandardMaterial::getColor() const
{
    auto *p = findParam(_Color);
    return p ? p->getFloat4() : float4::zero();
}
void StandardMaterial::setColorMap(int v)
{
    setColorMap(MakeTmpTexture(v));
}
void StandardMaterial::setColorMap(TexturePtr v)
{
    if (v && v->id != InvalidID)
        addParam({ _MainTex, v });
    else
        eraseParam(_MainTex);
}
int StandardMaterial::getColorMap() const
{
    auto *p = findParam("_MainTex");
    return p ? p->getTexture() : InvalidID;
}

void StandardMaterial::setEmissionColor(float4 v)
{
    addParam({ _EmissionColor, v });
}
float4 StandardMaterial::getEmissionColor() const
{
    auto *p = findParam(_EmissionColor);
    return p ? p->getFloat4() : float4::zero();
}
void StandardMaterial::setEmissionMap(int v)
{
    setEmissionMap(MakeTmpTexture(v));
}
void StandardMaterial::setEmissionMap(TexturePtr v)
{
    if (v && v->id != InvalidID)
        addParam({ _EmissionMap, v });
    else
        eraseParam(_EmissionMap);
}
int StandardMaterial::getEmissionMap() const
{
    auto *p = findParam(_EmissionMap);
    return p ? p->getTexture() : InvalidID;
}

void StandardMaterial::setMetallic(float v)
{
    addParam({ _Metallic, v });
}
float StandardMaterial::getMetallic() const
{
    auto *p = findParam(_Metallic);
    return p ? p->getFloat() : 0.0f;
}
void StandardMaterial::setMetallicMap(int v)
{
    setMetallicMap(MakeTmpTexture(v));
}
void StandardMaterial::setMetallicMap(TexturePtr v)
{
    if (v && v->id != InvalidID)
        addParam({ _MetallicGlossMap, v });
    else
        eraseParam(_MetallicGlossMap);
}
int StandardMaterial::getMetallicMap() const
{
    auto *p = findParam(_MetallicGlossMap);
    return p ? p->getTexture() : InvalidID;
}
void StandardMaterial::setSmoothness(float v)
{
    addParam({ _Glossiness, v });
}
float StandardMaterial::getSmoothness() const
{
    auto *p = findParam(_Glossiness);
    return p ? p->getFloat() : 0.0f;
}

void StandardMaterial::setBumpScale(float v)
{
    addParam({ _BumpScale, v });
}
float StandardMaterial::getBumpScale() const
{
    auto *p = findParam(_BumpScale);
    return p ? p->getFloat() : 0.0f;
}
void StandardMaterial::setBumpMap(int v)
{
    setBumpMap(MakeTmpTexture(v));
}
void StandardMaterial::setBumpMap(TexturePtr v)
{
    if (v && v->id != InvalidID)
        addParam({ _BumpMap, v });
    else
        eraseParam(_BumpMap);
}
int StandardMaterial::getBumpMap() const
{
    auto *p = findParam(_BumpMap);
    return p ? p->getTexture() : InvalidID;
}


// StandardSpecMaterial

static const char _SpecColor[] = "_SpecColor";
static const char _SpecGlossMap[] = "_SpecGlossMap";

void StandardSpecMaterial::setupShader()
{
    if (shader.empty())
        shader = "Standard (Specular setup)";
}

void StandardSpecMaterial::setSpecularColor(float4 v)
{
    setupShader();
    addParam({ _SpecColor, v });
}

float4 StandardSpecMaterial::getSpecularColor()
{
    auto *p = findParam(_SpecColor);
    return p ? p->getFloat4() : float4::zero();
}

void StandardSpecMaterial::setSpecularGlossMap(int v)
{
    setupShader();
    setSpecularGlossMap(MakeTmpTexture(v));
}

void StandardSpecMaterial::setSpecularGlossMap(TexturePtr v)
{
    setupShader();
    if (v && v->id != InvalidID)
        addParam({ _SpecGlossMap, v });
    else
        eraseParam(_SpecGlossMap);
}

int StandardSpecMaterial::getSpecularGlossMap()
{
    auto *p = findParam(_SpecGlossMap);
    return p ? p->getTexture() : InvalidID;
}

} // namespace ms
