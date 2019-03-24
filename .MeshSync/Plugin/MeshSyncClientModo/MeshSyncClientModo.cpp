#include "pch.h"
#include "MeshSyncClientModo.h"
#include "msmodoUtils.h"


MeshSyncClientModo& MeshSyncClientModo::getInstance()
{
    static MeshSyncClientModo s_instance;
    return s_instance;
}

MeshSyncClientModo::MeshSyncClientModo()
{
}

MeshSyncClientModo::~MeshSyncClientModo()
{
}

void MeshSyncClientModo::update()
{
}

bool MeshSyncClientModo::sendScene(SendScope scope, bool dirty_all)
{
    return false;
}

bool MeshSyncClientModo::sendAnimations(SendScope scope)
{
    return false;
}

bool MeshSyncClientModo::recvScene()
{
    return false;
}


// 
// component export
// 

ms::TransformPtr MeshSyncClientModo::exportObject(CLxLoc_Item& obj, bool force)
{
    return nullptr;
}

ms::TransformPtr MeshSyncClientModo::exportTransform(CLxLoc_Item& obj)
{
    auto ret = ms::Transform::create();

    //CLxUser_Scene      scene(SceneObject());
    //CLxUser_SceneGraph sceneGraph;
    //CLxUser_ItemGraph  itemGraph;
    //scene.GetGraph(LXsGRAPH_XFRMCORE, sceneGraph);
    //itemGraph.set(sceneGraph);

    //unsigned transformCount = itemGraph.Reverse(item);
    //for (unsigned transformIndex = 0; transformIndex < transformCount; ++transformIndex) {
    //    CLxUser_Item transform;
    //    if (itemGraph.Reverse(item, transformIndex, transform)) {
    //        SetItem(transform);

    //        const char *xformType = ItemType();
    //        if (LXTypeMatch(xformType, LXsITYPE_SCALE)) {
    //            // scale
    //            LXtVector scale;
    //            ChanXform(LXsICHAN_SCALE_SCL, scale);

    //        }
    //        else if (LXTypeMatch(xformType, LXsITYPE_ROTATION)) {
    //            // Rotation
    //            LXtVector rotate;
    //            ChanXform(LXsICHAN_ROTATION_ROT, rotate);
    //            int axis[3];
    //            buildAxisOrder(ChanInt(LXsICHAN_ROTATION_ORDER), axis);

    //        }
    //        else if (LXTypeMatch(xformType, LXsITYPE_TRANSLATION)) {
    //            // Translation
    //            LXtVector	translate;
    //            ChanXform(LXsICHAN_TRANSLATION_POS, translate);

    //        }
    //    }
    //}

    return ret;
}

ms::CameraPtr MeshSyncClientModo::exportCamera(CLxLoc_Item& obj)
{
    auto ret = ms::Camera::create();
    return ret;
}

ms::LightPtr MeshSyncClientModo::exportLight(CLxLoc_Item& obj)
{
    auto ret = ms::Light::create();
    return ret;
}

ms::MeshPtr MeshSyncClientModo::exportMesh(CLxLoc_Item& obj)
{
    auto ret = ms::Mesh::create();
    return ret;
}


// 
// animation export
// 

int MeshSyncClientModo::exportAnimations(SendScope scope)
{
    return 0;
}

bool MeshSyncClientModo::exportAnimation(CLxLoc_Item& obj, bool force)
{
    return false;
}

void MeshSyncClientModo::extractTransformAnimationData(ms::Animation& dst, CLxLoc_Item& obj)
{

}

void MeshSyncClientModo::extractCameraAnimationData(ms::Animation& dst, CLxLoc_Item& obj)
{

}

void MeshSyncClientModo::extractLightAnimationData(ms::Animation& dst, CLxLoc_Item& obj)
{

}

void MeshSyncClientModo::extractMeshAnimationData(ms::Animation& dst, CLxLoc_Item& obj)
{

}


void MeshSyncClientModo::kickAsyncSend()
{
    m_sender.on_prepare = [this]() {
        auto& t = m_sender;
        t.client_settings = m_settings.client_settings;
        t.scene_settings.handedness = ms::Handedness::Right;
        t.scene_settings.scale_factor = m_settings.scale_factor;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.animations = m_animations;

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();
    };
    m_sender.on_success = [this]() {
        m_material_manager.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
        m_animations.clear();
    };
    m_sender.kick();
}
