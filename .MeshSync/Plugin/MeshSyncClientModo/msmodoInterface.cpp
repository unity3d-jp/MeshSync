#include "pch.h"
#include "msmodoInterface.h"
#include "msmodoUtils.h"


class msmodoEventListener :
    public CLxImpl_SelectionListener,
    public CLxImpl_AnimListener,
    public CLxImpl_SceneItemListener,
    public CLxImpl_SceneEvalListener,
    public CLxSingletonPolymorph
{
public:
    LXxSINGLETON_METHOD;

    msmodoEventListener(msmodoInterface *ifs)
        : m_ifs(ifs)
    {
        AddInterface(new CLxIfc_SelectionListener<msmodoEventListener>());
        AddInterface(new CLxIfc_AnimListener<msmodoEventListener>());
        AddInterface(new CLxIfc_SceneItemListener<msmodoEventListener>());
        AddInterface(new CLxIfc_SceneEvalListener<msmodoEventListener>());
    }

    void selevent_Time(double time) override
    {
        // scrubbing also fire this event, but time value is unstable. so let it to animevent_ScrubTime().
        // this callback focus on handling playing state (between animevent_PlayStart() and animevent_PlayEnd())
        if (m_playing)
            m_ifs->onTimeChange();
    }
    LxResult animevent_TimeChange() override
    {
        // this callback seems never be fired...
        return LXe_OK;
    }
    LxResult animevent_PlayStart() override
    {
        m_playing = true;
        return LXe_OK;
    }
    LxResult animevent_PlayEnd() override
    {
        m_playing = false;
        return LXe_OK;
    }
    LxResult animevent_ScrubTime() override
    {
        m_ifs->onTimeChange();
        return LXe_OK;
    }
    LxResult animevent_ScrubEnd() override
    {
        return LXe_OK;
    }

private:
    msmodoInterface *m_ifs;
    bool m_playing = false;
};


void msmodoInterface::prepare()
{
    m_layer_service.SetScene(0);
    m_layer_service.Scene(m_current_scene);
    m_current_scene.GetChannels(m_ch_read, m_selection_service.GetTime());
    m_current_scene.GetSetupChannels(m_ch_read_setup);

    if (tMaterial == 0) {
        tMaterial = m_scene_service.ItemType(LXsITYPE_ADVANCEDMATERIAL);

        tLocator = m_scene_service.ItemType(LXsITYPE_LOCATOR);
        tCamera = m_scene_service.ItemType(LXsITYPE_CAMERA);
        tLight = m_scene_service.ItemType(LXsITYPE_LIGHT);
        tMesh = m_scene_service.ItemType(LXsITYPE_MESH);
        tMeshInst = m_scene_service.ItemType(LXsITYPE_MESHINST);

        tLightMaterial = m_scene_service.ItemType(LXsITYPE_LIGHTMATERIAL);
        tPointLight = m_scene_service.ItemType(LXsITYPE_POINTLIGHT);
        tDirectionalLight = m_scene_service.ItemType(LXsITYPE_SUNLIGHT);
        tSpotLight = m_scene_service.ItemType(LXsITYPE_SPOTLIGHT);
        tAreaLight = m_scene_service.ItemType(LXsITYPE_AREALIGHT);

        tDeform = m_scene_service.ItemType(LXsITYPE_DEFORM);
        tGenInf = m_scene_service.ItemType(LXsITYPE_GENINFLUENCE);
        tMorph = m_scene_service.ItemType(LXsITYPE_MORPHDEFORM);
    }
    if (!m_event_listener) {
        m_event_listener = new msmodoEventListener(this);
        m_listener_service.AddListener(*m_event_listener);
    }
}

void msmodoInterface::onTimeChange()
{
}


void msmodoInterface::setChannelReadTime(double time)
{
    if (time == std::numeric_limits<double>::infinity())
        time = m_selection_service.GetTime();
    m_current_scene.GetChannels(m_ch_read, time);
}



CLxUser_Mesh msmodoInterface::getBaseMesh(CLxUser_Item& obj)
{
    CLxUser_Mesh mesh;
    CLxUser_MeshFilter meshfilter;
    CLxUser_MeshFilterIdent meshfilter_ident;
    if (m_ch_read.Object(obj, LXsICHAN_MESH_MESH, meshfilter)) {
        if (meshfilter_ident.copy(meshfilter))
            meshfilter_ident.GetMesh(LXs_MESHFILTER_BASE, mesh);
    }
    return mesh;
}

CLxUser_Mesh msmodoInterface::getDeformedMesh(CLxUser_Item& obj)
{
    CLxUser_Mesh mesh;
    CLxUser_MeshFilter meshfilter;
    if (m_ch_read.Object(obj, LXsICHAN_MESH_MESH, meshfilter)) {
        meshfilter.GetMesh(mesh);
    }
    return mesh;
}

std::tuple<double, double> msmodoInterface::getTimeRange()
{
    double start = 0.0, end = 1.0;
    {
        CLxReadPreferenceValue q;
        q.Query(LXsPREFERENCE_VALUE_ANIMATION_TIME_RANGE_START);
        start = q.GetFloat();
    }
    {
        CLxReadPreferenceValue q;
        q.Query(LXsPREFERENCE_VALUE_ANIMATION_TIME_RANGE_END);
        end = q.GetFloat();
    }
    return { start, end };
}

void msmodoInterface::dbgDumpItem(CLxUser_Item item)
{
    {
        auto path = GetPath(item);
        auto t = item.Type();
        const char *tname;
        m_scene_service.ItemTypeName(t, &tname);
        msLogInfo("%s (%s)\n", path.c_str(), tname);
    }

    uint32_t n;
    item.ChannelCount(&n);
    for (uint32_t i = 0; i < n; ++i) {
        const char *name, *tname;
        uint32_t type;
        item.ChannelName(i, &name);
        item.ChannelType(i, &type);
        m_scene_service.ItemTypeName(type, &tname);
        msLogInfo(" - %s (%s)\n", name, tname);
    }
}



uint32_t mdmodoSkinDeformer::ch_enable;
uint32_t mdmodoSkinDeformer::ch_type;
uint32_t mdmodoSkinDeformer::ch_mapname;

mdmodoSkinDeformer::mdmodoSkinDeformer(msmodoInterface& ifs, CLxUser_Item& item)
    : m_ifs(ifs), m_item(item)
{
    if (ch_enable == 0) {
        m_item.ChannelLookup(LXsICHAN_GENINFLUENCE_ENABLE, &ch_enable);
        m_item.ChannelLookup(LXsICHAN_GENINFLUENCE_TYPE, &ch_type);
        m_item.ChannelLookup(LXsICHAN_GENINFLUENCE_NAME, &ch_mapname);
    }
}

bool mdmodoSkinDeformer::isEnabled()
{
    int ret;
    m_ifs.m_ch_read.Integer(m_item, ch_enable, &ret);
    return ret != 0;
}

int mdmodoSkinDeformer::getInfluenceType()
{
    int ret;
    m_ifs.m_ch_read.Integer(m_item, ch_type, &ret);
    return ret;
}

const char* mdmodoSkinDeformer::getMapName()
{
    const char *ret;
    m_ifs.m_ch_read.String(m_item, ch_mapname, &ret);
    return ret;
}

CLxUser_Item mdmodoSkinDeformer::getEffector()
{
    CLxUser_Item effector;
    if (LXx_FAIL(m_ifs.m_deform_service.DeformerDeformationItem(m_item, effector)))
        return nullptr;
    return effector;
}



uint32_t mdmodoMorphDeformer::ch_enable;
uint32_t mdmodoMorphDeformer::ch_strength;
uint32_t mdmodoMorphDeformer::ch_mapname;

mdmodoMorphDeformer::mdmodoMorphDeformer(msmodoInterface& ifs, CLxUser_Item& item)
    : m_ifs(ifs), m_item(item)
{
    if (ch_enable == 0) {
        m_item.ChannelLookup(LXsICHAN_MORPHDEFORM_ENABLE, &ch_enable);
        m_item.ChannelLookup(LXsICHAN_MORPHDEFORM_STRENGTH, &ch_strength);
        m_item.ChannelLookup(LXsICHAN_MORPHDEFORM_MAPNAME, &ch_mapname);
    }
}

bool mdmodoMorphDeformer::isEnabled()
{
    int ret;
    m_ifs.m_ch_read.Integer(m_item, ch_enable, &ret);
    return ret != 0;
}

float mdmodoMorphDeformer::getWeight()
{
    double ret;
    m_ifs.m_ch_read.Double(m_item, ch_strength, &ret);
    return (float)ret * 100.0f;
}

const char* mdmodoMorphDeformer::getMapName()
{
    const char *ret;
    m_ifs.m_ch_read.String(m_item, ch_mapname, &ret);
    return ret;
}
