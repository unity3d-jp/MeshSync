#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/SceneGraph/msMaterial.h"

msDeclClassPtr(Material)

namespace ms {
/**
 * Generic PBR shader using field names from Unity's Standard shader.
 */
class StandardMaterial : public Material
{
public:
    void    setColor(mu::float4 v);
    mu::float4  getColor() const;
    void    setColorMap(const TextureRecord& v);
    void    setColorMap(const TexturePtr v);
    TextureRecord* getColorMap() const;

    void    SetDetailAlbedoMap(const TextureRecord& v);
    void    SetDetailAlbedoMap(const TexturePtr v);
    TextureRecord* GetDetailAlbedoMap() const;

    void    SetUVForSecondaryMap(float v);
    float   GetUVForSecondaryMap() const;

    void    setEmissionColor(mu::float4 v);
    mu::float4  getEmissionColor() const;
    void    setEmissionMap(const TextureRecord& v);
    void    setEmissionMap(TexturePtr v);
    TextureRecord* getEmissionMap() const;

    void    setEmissionStrength(float v);

    void    setMetallic(float v);
    float   getMetallic() const;
    void    setMetallicMap(const TextureRecord& v);
    void    setMetallicMap(TexturePtr v);
    TextureRecord* getMetallicMap() const;
    void    setSmoothness(float v);
    void    setSmoothnessMap(const TextureRecord& v);
    void    setRoughnessMap(const TextureRecord& v);
    float   getSmoothness() const;
    void    setOcclusionMap(const TextureRecord& v);

    void    setBumpScale(float v);
    float   getBumpScale() const;
    void    setBumpMap(const TextureRecord& v);
    void    setBumpMap(TexturePtr v);
    TextureRecord* getBumpMap() const;

    void    setHeightScale(float v);
    void    setHeightMap(const TextureRecord& v);

    void    setSpecular(mu::float3 v);

    void    setClearCoat(float v);
    void    setClearCoatMask(const TextureRecord& v);
    
    void    setShader(const std::string shaderName);
};
msDeclPtr(StandardMaterial);
inline StandardMaterial& AsStandardMaterial(Material& p) { return reinterpret_cast<StandardMaterial&>(p); }
inline StandardMaterial* AsStandardMaterial(Material* p) { return reinterpret_cast<StandardMaterial*>(p); }
inline StandardMaterialPtr AsStandardMaterial(MaterialPtr p) { return std::static_pointer_cast<StandardMaterial>(p); }

//----------------------------------------------------------------------------------------------------------------------

class StandardSpecMaterial : public StandardMaterial
{
public:
    void    setSpecularColor(mu::float4 v);
    mu::float4  getSpecularColor();
    void    setSpecularGlossMap(const TextureRecord& v);
    void    setSpecularGlossMap(TexturePtr v);
    TextureRecord* getSpecularGlossMap();
private:
    void setupShader();
};
msDeclPtr(StandardSpecMaterial);
inline StandardSpecMaterial& AsStandardSpecMaterial(Material& p) { return reinterpret_cast<StandardSpecMaterial&>(p); }
inline StandardSpecMaterial* AsStandardSpecMaterial(Material* p) { return reinterpret_cast<StandardSpecMaterial*>(p); }
inline StandardSpecMaterialPtr AsStandardSpecMaterial(MaterialPtr p) { return std::static_pointer_cast<StandardSpecMaterial>(p); }

} // namespace ms
