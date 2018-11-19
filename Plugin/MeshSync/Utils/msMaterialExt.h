#pragma once

#include "../msMaterial.h"

namespace ms {

// correspond to Unity's Standard shader
class StandardMaterial : public Material
{
public:
    void    setColor(float4 v);
    float4  getColor() const;
    void    setColorMap(int v);
    void    setColorMap(TexturePtr v);
    int     getColorMap() const;

    void    setEmissionColor(float4 v);
    float4  getEmissionColor() const;
    void    setEmissionMap(int v);
    void    setEmissionMap(TexturePtr v);
    int     getEmissionMap() const;

    void    setMetallic(float v);
    float   getMetallic() const;
    void    setMetallicMap(int v);
    void    setMetallicMap(TexturePtr v);
    int     getMetallicMap() const;
    void    setSmoothness(float v);
    float   getSmoothness() const;

    void    setBumpScale(float v);
    float   getBumpScale() const;
    void    setBumpMap(int v);
    void    setBumpMap(TexturePtr v);
    int     getBumpMap() const;
};
using StandardMaterialPtr = std::shared_ptr<StandardMaterial>;
inline StandardMaterial& AsStandardMaterial(Material& p) { return static_cast<StandardMaterial&>(p); }
inline StandardMaterial* AsStandardMaterial(Material* p) { return static_cast<StandardMaterial*>(p); }
inline StandardMaterialPtr AsStandardMaterial(MaterialPtr p) { return std::static_pointer_cast<StandardMaterial>(p); }


class StandardSpecMaterial : public StandardMaterial
{
public:
    void    setSpecularColor(float4 v);
    float4  getSpecularColor();
    void    setSpecularGlossMap(int v);
    void    setSpecularGlossMap(TexturePtr v);
    int     getSpecularGlossMap();
private:
    void setupShader();
};
using StandardSpecMaterialPtr = std::shared_ptr<StandardSpecMaterial>;
inline StandardSpecMaterial& AsStandardSpecMaterial(Material& p) { return static_cast<StandardSpecMaterial&>(p); }
inline StandardSpecMaterial* AsStandardSpecMaterial(Material* p) { return static_cast<StandardSpecMaterial*>(p); }
inline StandardSpecMaterialPtr AsStandardSpecMaterial(MaterialPtr p) { return std::static_pointer_cast<StandardSpecMaterial>(p); }

} // namespace ms
