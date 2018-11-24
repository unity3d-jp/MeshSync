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

using TextureRecord = MaterialProperty::TextureRecord;

void StandardMaterial::setColor(float4 v)
{
    addProperty({ _Color, v });
}
float4 StandardMaterial::getColor() const
{
    auto *p = findProperty(_Color);
    return p ? p->get<float4>() : float4::zero();
}
void StandardMaterial::setColorMap(const TextureRecord& v)
{
    addProperty({ _MainTex,v });
}
void StandardMaterial::setColorMap(TexturePtr v)
{
    if (v)
        addProperty({ _MainTex, v });
}
Material::TextureRecord* StandardMaterial::getColorMap() const
{
    auto *p = findProperty("_MainTex");
    return p ? &p->get<TextureRecord>() : nullptr;
}

void StandardMaterial::setEmissionColor(float4 v)
{
    addProperty({ _EmissionColor, v });
}
float4 StandardMaterial::getEmissionColor() const
{
    auto *p = findProperty(_EmissionColor);
    return p ? p->get<float4>() : float4::zero();
}
void StandardMaterial::setEmissionMap(const TextureRecord& v)
{
    addProperty({ _EmissionMap, v });
}
void StandardMaterial::setEmissionMap(TexturePtr v)
{
    if (v)
        addProperty({ _EmissionMap, v });
}
Material::TextureRecord* StandardMaterial::getEmissionMap() const
{
    auto *p = findProperty(_EmissionMap);
    return p ? &p->get<TextureRecord>() : nullptr;
}

void StandardMaterial::setMetallic(float v)
{
    addProperty({ _Metallic, v });
}
float StandardMaterial::getMetallic() const
{
    auto *p = findProperty(_Metallic);
    return p ? p->get<float>() : 0.0f;
}
void StandardMaterial::setMetallicMap(const TextureRecord& v)
{
    addProperty({ _MetallicGlossMap, v });
}
void StandardMaterial::setMetallicMap(TexturePtr v)
{
    if (v)
        addProperty({ _MetallicGlossMap, v });
}
Material::TextureRecord* StandardMaterial::getMetallicMap() const
{
    auto *p = findProperty(_MetallicGlossMap);
    return p ? &p->get<TextureRecord>() : nullptr;
}
void StandardMaterial::setSmoothness(float v)
{
    addProperty({ _Glossiness, v });
}
float StandardMaterial::getSmoothness() const
{
    auto *p = findProperty(_Glossiness);
    return p ? p->get<float>() : 0.0f;
}

void StandardMaterial::setBumpScale(float v)
{
    addProperty({ _BumpScale, v });
}
float StandardMaterial::getBumpScale() const
{
    auto *p = findProperty(_BumpScale);
    return p ? p->get<float>() : 0.0f;
}
void StandardMaterial::setBumpMap(const TextureRecord& v)
{
    addProperty({ _BumpMap, v });
}
void StandardMaterial::setBumpMap(TexturePtr v)
{
    if (v)
        addProperty({ _BumpMap, v });
}
Material::TextureRecord* StandardMaterial::getBumpMap() const
{
    auto *p = findProperty(_BumpMap);
    return p ? &p->get<TextureRecord>() : nullptr;
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
    addProperty({ _SpecColor, v });
}

float4 StandardSpecMaterial::getSpecularColor()
{
    auto *p = findProperty(_SpecColor);
    return p ? p->get<float4>() : float4::zero();
}

void StandardSpecMaterial::setSpecularGlossMap(const TextureRecord& v)
{
    setupShader();
    addProperty({ _SpecGlossMap, v });
}

void StandardSpecMaterial::setSpecularGlossMap(TexturePtr v)
{
    setupShader();
    if (v)
        addProperty({ _SpecGlossMap, v });
}

Material::TextureRecord* StandardSpecMaterial::getSpecularGlossMap()
{
    auto *p = findProperty(_SpecGlossMap);
    return p ? &p->get<TextureRecord>() : nullptr;
}

} // namespace ms
