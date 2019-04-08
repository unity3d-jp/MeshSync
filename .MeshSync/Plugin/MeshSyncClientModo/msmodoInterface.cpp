#include "pch.h"
#include "msmodoInterface.h"
#include "msmodoUtils.h"
#include "msmodoCompat.h"


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
        // scrubbing also fire this callback, but time value is unstable. so let it to animevent_ScrubTime().
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

    void sil_ItemAdd(ILxUnknownID item) override
    {
        CLxUser_Item obj(item);
        if (obj.IsA(m_ifs->tLocator))
            m_ifs->onItemAdd(obj);
    }

    void sil_ItemRemove(ILxUnknownID item) override
    {
        CLxUser_Item obj(item);
        if (obj.IsA(m_ifs->tLocator))
            m_ifs->onItemRemove(obj);
    }

    void sil_ItemParent(ILxUnknownID item) override
    {
        CLxUser_Item obj(item);
        if (obj.IsA(m_ifs->tLocator))
            m_ifs->onTreeRestructure();
    }

    void sil_ItemChild(ILxUnknownID item) override
    {
        CLxUser_Item obj(item);
        if (obj.IsA(m_ifs->tLocator))
            m_ifs->onTreeRestructure();
    }

    void sil_ItemName(ILxUnknownID item) override
    {
        CLxUser_Item obj(item);
        if (obj.IsA(m_ifs->tLocator))
            m_ifs->onTreeRestructure();
    }

    void sel_ChannelValue(ILxUnknownID item, unsigned index) override
    {
        CLxUser_Item obj(item);
        if (obj.IsA(m_ifs->tLocator))
            m_ifs->onItemUpdate(obj);
    }

private:
    msmodoInterface *m_ifs;
    bool m_playing = false;
};

class msmodoTimerCallbackVisitor : public CLxVisitor
{
public:
    msmodoTimerCallbackVisitor(msmodoInterface *ifs) : m_ifs(ifs) {}

    void evaluate() override
    {
        m_ifs->onIdle();
        m_ifs->startTimer();
    }

    msmodoInterface *m_ifs;
};


msmodoInterface::msmodoInterface()
{
}

msmodoInterface::~msmodoInterface()
{
    if (m_timer_callback) {
        delete m_timer_callback;
        m_timer_callback = nullptr;
    }
}

void msmodoInterface::prepare()
{
    m_svc_layer.SetScene(0);
    m_svc_layer.Scene(m_current_scene);
    m_current_scene.GetChannels(m_ch_read, m_svc_selection.GetTime());
    m_current_scene.GetSetupChannels(m_ch_read_setup);

    CLxUser_SceneGraph scene_graph;
    m_current_scene.GetGraph(LXsGRAPH_SHADELOC, scene_graph);
    if (scene_graph)
        m_shadeloc_graph.set(scene_graph);

    if (tMaterial == 0) {
        tMaterial = m_svc_scene.ItemType(LXsITYPE_ADVANCEDMATERIAL);

        tLocator = m_svc_scene.ItemType(LXsITYPE_LOCATOR);
        tCamera = m_svc_scene.ItemType(LXsITYPE_CAMERA);
        tLight = m_svc_scene.ItemType(LXsITYPE_LIGHT);
        tMesh = m_svc_scene.ItemType(LXsITYPE_MESH);
        tMeshInst = m_svc_scene.ItemType(LXsITYPE_MESHINST);
        tReplicator = m_svc_scene.ItemType(LXsITYPE_REPLICATOR);

        tLightMaterial = m_svc_scene.ItemType(LXsITYPE_LIGHTMATERIAL);
        tPointLight = m_svc_scene.ItemType(LXsITYPE_POINTLIGHT);
        tDirectionalLight = m_svc_scene.ItemType(LXsITYPE_SUNLIGHT);
        tSpotLight = m_svc_scene.ItemType(LXsITYPE_SPOTLIGHT);
        tAreaLight = m_svc_scene.ItemType(LXsITYPE_AREALIGHT);

        tDeform = m_svc_scene.ItemType(LXsITYPE_DEFORM);
        tGenInf = m_svc_scene.ItemType(LXsITYPE_GENINFLUENCE);
        tMorph = m_svc_scene.ItemType(LXsITYPE_MORPHDEFORM);

        tImageMap = m_svc_scene.ItemType(LXsITYPE_IMAGEMAP);
        tTextureLayer = m_svc_scene.ItemType(LXsITYPE_TEXTURELAYER);
        tVideoStill = m_svc_scene.ItemType(LXsITYPE_VIDEOSTILL);
    }
    if (!m_event_listener) {
        m_event_listener = new msmodoEventListener(this);
        m_svc_listener.AddListener(*m_event_listener);
    }
    if (!m_timer_callback) {
        m_timer_callback = new msmodoTimerCallbackVisitor(this);
        startTimer();
    }
}

void msmodoInterface::startTimer()
{
    if (m_timer_callback)
        m_svc_platform.TimerStart(*m_timer_callback, 50, USERIDLEf_ALL_IDLE);
}


void msmodoInterface::onItemAdd(CLxUser_Item& item) {}
void msmodoInterface::onItemRemove(CLxUser_Item& item) {}
void msmodoInterface::onItemUpdate(CLxUser_Item& item) {}
void msmodoInterface::onTreeRestructure() {}
void msmodoInterface::onTimeChange() {}
void msmodoInterface::onIdle() {}



void msmodoInterface::setChannelReadTime(double time)
{
    if (time == std::numeric_limits<double>::infinity())
        time = m_svc_selection.GetTime();
    m_current_scene.GetChannels(m_ch_read, time);
}


template<class MeshFilterType>
static inline void GetBaseMeshImpl(CLxUser_ChannelRead& ch_read, CLxUser_Item& obj, CLxUser_Mesh& dst_mesh)
{
    MeshFilterType meshfilter;
    CLxUser_MeshFilterIdent meshfilter_ident;
    if (ch_read.Object(obj, LXsICHAN_MESH_MESH, meshfilter)) {
        if (meshfilter_ident.copy(meshfilter))
            meshfilter_ident.GetMesh(LXs_MESHFILTER_BASE, dst_mesh);
    }
}

template<class MeshFilterType>
static inline void GetDeformedMeshImpl(CLxUser_ChannelRead& ch_read, CLxUser_Item& obj, CLxUser_Mesh& dst_mesh)
{
    MeshFilterType meshfilter;
    if (ch_read.Object(obj, LXsICHAN_MESH_MESH, meshfilter)) {
        meshfilter.GetMesh(dst_mesh);
    }
}

CLxUser_Mesh msmodoInterface::getBaseMesh(CLxUser_Item& obj)
{
    if (!obj.IsA(tMesh))
        return nullptr;

    CLxUser_Mesh mesh;
    GetBaseMeshImpl<CLxUser_MeshFilter>(m_ch_read, obj, mesh);
    if (!mesh)
        GetBaseMeshImpl<CLxUser_MeshFilter1>(m_ch_read, obj, mesh);
    return mesh;
}

CLxUser_Mesh msmodoInterface::getDeformedMesh(CLxUser_Item& obj)
{
    if (!obj.IsA(tMesh))
        return nullptr;

    CLxUser_Mesh mesh;
    GetDeformedMeshImpl<CLxUser_MeshFilter>(m_ch_read, obj, mesh);
    if (!mesh)
        GetDeformedMeshImpl<CLxUser_MeshFilter1>(m_ch_read, obj, mesh);
    return mesh;
}

const char* msmodoInterface::getImageFilePath(CLxUser_Item& image)
{
    CLxUser_Item still;
    int n = m_shadeloc_graph.Forward(image);
    for (int i = 0; i < n; i++) {
        m_shadeloc_graph.Forward(image, i, still);
        const char *filename;
        if (still.IsA(tVideoStill) && m_ch_read.Get(still, LXsICHAN_VIDEOSTILL_FILENAME, &filename))
            return filename;
    }
    return nullptr;
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
    if (!item)
        return;

    {
        auto path = GetPath(item);
        auto t = item.Type();
        const char *tname;
        m_svc_scene.ItemTypeName(t, &tname);
        msLogInfo("%s (%s)\n", path.c_str(), tname);
    }

    uint32_t n;
    item.ChannelCount(&n);
    for (uint32_t i = 0; i < n; ++i) {
        const char *name, *tname;
        uint32_t ch;
        item.ChannelName(i, &name);
        item.ChannelLookup(name, &ch);
        m_ch_read.TypeName(item, ch, &tname);
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
    if (LXx_FAIL(m_ifs.m_svc_deform.DeformerDeformationItem(m_item, effector)))
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
