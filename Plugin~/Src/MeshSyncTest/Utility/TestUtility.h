#pragma once
#include "MeshSync/msClient.h"  //ms::ClientSettings
#include "MeshSync/SceneGraph/msAsset.h" //ms::TexturePtr

#include "MeshSync/SceneGraph/msTexture.h"

class TestUtility {
public:

    template<class color_t>
    static void CreateCheckerImage(SharedVector<char>& dst, color_t black, color_t white, const int width, const int height)
    {
        const int num_pixels = width * height;
        const int checker_size = 8;
        dst.resize_discard(num_pixels * sizeof(color_t));
        color_t *data = (color_t*)dst.data();
        for (int iy = 0; iy < height; ++iy) {
            for (int ix = 0; ix < width; ++ix) {
                const bool cy = (iy / checker_size) % 2 == 0;
                bool cx = (ix / checker_size) % 2 == 0;
                if (cy)
                    *data++ = cx ? white : black;
                else
                    *data++ = cx ? black : white;
            }
        }
    }

    template<class color_t>
    static ms::TexturePtr CreateCheckerImageTexture(color_t black, color_t white, int width, int height, int id, const char *name)
    {
        std::shared_ptr<ms::Texture> tex = ms::Texture::create();
        tex->id = id;
        tex->name = name;
        tex->format = ms::GetTextureFormat<color_t>::result;
        tex->width = width;
        tex->height = height;
        CreateCheckerImage(tex->data, black, white, width, height);
        return tex;
    }

    static ms::ClientSettings GetClientSettings();
    static void Send(ms::ScenePtr scene);


};
