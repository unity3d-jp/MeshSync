#pragma once

#include "../msMaterial.h"

namespace ms {
    
// correspond to Unity's Standard shader
class StandardMaterial : public Material
{
public:
    void    setColor(float4 v);
    float4  getColor() const;
    void    setEmission(float4 v);
    float4  getEmission() const;
    void    setMetallic(float v);
    float   getMetallic() const;
    void    setSmoothness(float v);
    float   getSmoothness() const;

    void    setColorMap(int v);
    void    setColorMap(TexturePtr v);
    int     getColorMap() const;
    void    setEmissionMap(int v);
    void    setEmissionMap(TexturePtr v);
    int     getEmissionMap() const;
    void    setMetallicMap(int v);
    void    setMetallicMap(TexturePtr v);
    int     getMetallicMap() const;
    void    setNormalMap(int v);
    void    setNormalMap(TexturePtr v);
    int     getNormalMap() const;
};
using StandardMaterialPtr = std::shared_ptr<StandardMaterial>;

inline StandardMaterial& AsStandardMaterial(Material& p) { return static_cast<StandardMaterial&>(p); }
inline StandardMaterial* AsStandardMaterial(Material* p) { return static_cast<StandardMaterial*>(p); }
inline StandardMaterialPtr AsStandardMaterial(MaterialPtr p) { return std::static_pointer_cast<StandardMaterial>(p); }

} // namespace ms
