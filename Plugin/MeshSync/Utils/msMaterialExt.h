#pragma once

#include "SceneGraph/msMaterial.h"

namespace ms {

// correspond to Unity's Standard shader
class StandardMaterial : public Material
{
public:
    void    setColor(float4 v);
    float4  getColor() const;
    void    setColorMap(const TextureRecord& v);
    void    setColorMap(TexturePtr v);
    TextureRecord* getColorMap() const;

    void    setEmissionColor(float4 v);
    float4  getEmissionColor() const;
    void    setEmissionMap(const TextureRecord& v);
    void    setEmissionMap(TexturePtr v);
    TextureRecord* getEmissionMap() const;

    void    setMetallic(float v);
    float   getMetallic() const;
    void    setMetallicMap(const TextureRecord& v);
    void    setMetallicMap(TexturePtr v);
    TextureRecord* getMetallicMap() const;
    void    setSmoothness(float v);
    float   getSmoothness() const;

    void    setBumpScale(float v);
    float   getBumpScale() const;
    void    setBumpMap(const TextureRecord& v);
    void    setBumpMap(TexturePtr v);
    TextureRecord* getBumpMap() const;
};
msDeclPtr(StandardMaterial);
inline StandardMaterial& AsStandardMaterial(Material& p) { return static_cast<StandardMaterial&>(p); }
inline StandardMaterial* AsStandardMaterial(Material* p) { return static_cast<StandardMaterial*>(p); }
inline StandardMaterialPtr AsStandardMaterial(MaterialPtr p) { return std::static_pointer_cast<StandardMaterial>(p); }


class StandardSpecMaterial : public StandardMaterial
{
public:
    void    setSpecularColor(float4 v);
    float4  getSpecularColor();
    void    setSpecularGlossMap(const TextureRecord& v);
    void    setSpecularGlossMap(TexturePtr v);
    TextureRecord* getSpecularGlossMap();
private:
    void setupShader();
};
msDeclPtr(StandardSpecMaterial);
inline StandardSpecMaterial& AsStandardSpecMaterial(Material& p) { return static_cast<StandardSpecMaterial&>(p); }
inline StandardSpecMaterial* AsStandardSpecMaterial(Material* p) { return static_cast<StandardSpecMaterial*>(p); }
inline StandardSpecMaterialPtr AsStandardSpecMaterial(MaterialPtr p) { return std::static_pointer_cast<StandardSpecMaterial>(p); }

} // namespace ms
