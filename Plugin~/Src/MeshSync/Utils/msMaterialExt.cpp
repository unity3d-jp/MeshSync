#include "pch.h"
#include "MeshSync/Utility/msMaterialExt.h"

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
static const char DETAIL_ALBEDO_MAP_SHADER_VAR[] = "_DetailAlbedoMap";
static const char UV_SEC_SHADER_VAR[] = "_UVSec";

using TextureRecord = MaterialProperty::TextureRecord;

void StandardMaterial::setColor(mu::float4 v)
{
    addProperty( MaterialProperty( _Color, v ));
}
mu::float4 StandardMaterial::getColor() const
{
    const MaterialProperty* p = findProperty(_Color);
    return p ? p->get<mu::float4>() : mu::float4::zero();
}
void StandardMaterial::setColorMap(const TextureRecord& v)
{
    addProperty( MaterialProperty(_MainTex,v));
}
void StandardMaterial::setColorMap(const TexturePtr v)
{
    if (v)
        addProperty(MaterialProperty(_MainTex, v ));
}

Material::TextureRecord* StandardMaterial::getColorMap() const {
    const MaterialProperty* p = findProperty(_MainTex);
    return p ? &p->get<TextureRecord>() : nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void StandardMaterial::SetDetailAlbedoMap(const TextureRecord& v) {
    addProperty( MaterialProperty(DETAIL_ALBEDO_MAP_SHADER_VAR,v));
}

void StandardMaterial::SetDetailAlbedoMap(TexturePtr v) {
    assert(v);
    addProperty( MaterialProperty(DETAIL_ALBEDO_MAP_SHADER_VAR,v));

}
TextureRecord* StandardMaterial::GetDetailAlbedoMap() const {
    const MaterialProperty* p = findProperty(DETAIL_ALBEDO_MAP_SHADER_VAR);
    return p ? &p->get<TextureRecord>() : nullptr;

}

//----------------------------------------------------------------------------------------------------------------------

void  StandardMaterial::SetUVForSecondaryMap(float v) {
    addProperty(MaterialProperty( UV_SEC_SHADER_VAR, v ));
}
float StandardMaterial::GetUVForSecondaryMap() const {
    const MaterialProperty* p = findProperty(UV_SEC_SHADER_VAR);
    return p ? p->get<float>() : 0.0f;
}

//----------------------------------------------------------------------------------------------------------------------

void StandardMaterial::setEmissionColor(mu::float4 v)
{
    addProperty(MaterialProperty( _EmissionColor, v ));
}
mu::float4 StandardMaterial::getEmissionColor() const
{
    const MaterialProperty* p = findProperty(_EmissionColor);
    return p ? p->get<mu::float4>() : mu::float4::zero();
}
void StandardMaterial::setEmissionMap(const TextureRecord& v){
    addProperty(MaterialProperty(_EmissionMap, v ));
}

void StandardMaterial::setEmissionMap(TexturePtr v) {
    if (v)
        addProperty(MaterialProperty( _EmissionMap, v ));
}
Material::TextureRecord* StandardMaterial::getEmissionMap() const
{
    const MaterialProperty* p = findProperty(_EmissionMap);
    return p ? &p->get<TextureRecord>() : nullptr;
}

void StandardMaterial::setMetallic(float v) {
    addProperty( MaterialProperty( _Metallic, v ));
}
float StandardMaterial::getMetallic() const
{
    const MaterialProperty* p = findProperty(_Metallic);
    return p ? p->get<float>() : 0.0f;
}
void StandardMaterial::setMetallicMap(const TextureRecord& v) {
    addProperty( MaterialProperty( _MetallicGlossMap, v ));
}
void StandardMaterial::setMetallicMap(TexturePtr v) {
    if (v)
        addProperty(MaterialProperty( _MetallicGlossMap, v ));
}
Material::TextureRecord* StandardMaterial::getMetallicMap() const
{
    const MaterialProperty* p = findProperty(_MetallicGlossMap);
    return p ? &p->get<TextureRecord>() : nullptr;
}
void StandardMaterial::setSmoothness(float v)
{
    addProperty(MaterialProperty( _Glossiness, v ));
}
float StandardMaterial::getSmoothness() const
{
    const MaterialProperty* p = findProperty(_Glossiness);
    return p ? p->get<float>() : 0.0f;
}

void StandardMaterial::setBumpScale(float v)
{
    addProperty(MaterialProperty( _BumpScale, v ));
}
float StandardMaterial::getBumpScale() const
{
    const MaterialProperty* p = findProperty(_BumpScale);
    return p ? p->get<float>() : 0.0f;
}
void StandardMaterial::setBumpMap(const TextureRecord& v) {
    addProperty(MaterialProperty( _BumpMap, v ));
}
void StandardMaterial::setBumpMap(const TexturePtr v) {
    if (v)
        addProperty(MaterialProperty( _BumpMap, v ));
}
Material::TextureRecord* StandardMaterial::getBumpMap() const
{
    const MaterialProperty* p = findProperty(_BumpMap);
    return p ? &p->get<TextureRecord>() : nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

// StandardSpecMaterial

static const char _SpecColor[] = "_SpecColor";
static const char _SpecGlossMap[] = "_SpecGlossMap";

void StandardSpecMaterial::setupShader() {
    if (shader.empty())
        shader = "Standard (Specular setup)";
}

void StandardSpecMaterial::setSpecularColor(mu::float4 v) {
    setupShader();
    addProperty(MaterialProperty( _SpecColor, v ));
}

mu::float4 StandardSpecMaterial::getSpecularColor() {
    MaterialProperty* p = findProperty(_SpecColor);
    return p ? p->get<mu::float4>() : mu::float4::zero();
}

void StandardSpecMaterial::setSpecularGlossMap(const TextureRecord& v) {
    setupShader();
    addProperty(MaterialProperty( _SpecGlossMap, v ));
}

void StandardSpecMaterial::setSpecularGlossMap(TexturePtr v) {
    setupShader();
    if (v)
        addProperty(MaterialProperty( _SpecGlossMap, v ));
}

Material::TextureRecord* StandardSpecMaterial::getSpecularGlossMap()
{
    MaterialProperty* p = findProperty(_SpecGlossMap);
    return p ? &p->get<TextureRecord>() : nullptr;
}

} // namespace ms
