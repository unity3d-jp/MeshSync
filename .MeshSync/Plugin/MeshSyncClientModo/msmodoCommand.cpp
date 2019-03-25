#include "pch.h"
#include "MeshSyncClientModo.h"
#include "msmodoUtils.h"
#include "msmodoCommand.h"


class msmodoCmdSettings : public CLxCommand
{
public:
    bool getArg(const char *name, bool& dst)
    {
        int tmp;
        if (cmd_read_arg(name, tmp)) {
            dst = tmp != 0;
            return true;
        }
        return false;
    }
    bool getArg(const char *name, int& dst)
    {
        if (cmd_read_arg(name, dst)) {
            return true;
        }
        return false;
    }
    bool getArg(const char *name, uint16_t& dst)
    {
        double tmp;
        if (cmd_read_arg(name, tmp)) {
            dst = (uint16_t)tmp;
            return true;
        }
        return false;
    }
    bool getArg(const char *name, float& dst)
    {
        double tmp;
        if (cmd_read_arg(name, tmp)) {
            dst = (float)tmp;
            return true;
        }
        return false;
    }
    bool getArg(const char *name, std::string& dst)
    {
        if (cmd_read_arg(name, dst)) {
            return true;
        }
        return false;
    }

    void setArg(const char *name, bool v) { cmd_set_arg(name, (int)v); }
    void setArg(const char *name, int v) { cmd_set_arg(name, v); }
    void setArg(const char *name, uint16_t v) { cmd_set_arg(name, v); }
    void setArg(const char *name, float v) { cmd_set_arg(name, (double)v); }
    void setArg(const char *name, const std::string& v) { cmd_set_arg(name, v); }

#define EachParams(Handler)\
    Handler("address", LXsTYPE_STRING, settings.client_settings.server, false)\
    Handler("port", LXsTYPE_INTEGER, settings.client_settings.port, false)\
    Handler("scaleFactor", LXsTYPE_FLOAT, settings.scale_factor, true)\
    Handler("autosync", LXsTYPE_BOOLEAN, settings.auto_sync, true)\
    Handler("syncMeshes", LXsTYPE_BOOLEAN, settings.sync_meshes, true)\
    Handler("syncNormals", LXsTYPE_BOOLEAN, settings.sync_normals, true)\
    Handler("syncUVs", LXsTYPE_BOOLEAN, settings.sync_uvs, true)\
    Handler("syncColors", LXsTYPE_BOOLEAN, settings.sync_colors, true)\
    Handler("makeDoubleSided", LXsTYPE_BOOLEAN, settings.make_double_sided, true)\
    Handler("bakeDeformers", LXsTYPE_BOOLEAN, settings.bake_deformers, true)\
    Handler("syncBlendShapes", LXsTYPE_BOOLEAN, settings.sync_blendshapes, true)\
    Handler("syncBones", LXsTYPE_BOOLEAN, settings.sync_bones, true)\
    Handler("syncTextures", LXsTYPE_BOOLEAN, settings.sync_textures, true)\
    Handler("syncCameras", LXsTYPE_BOOLEAN, settings.sync_cameras, true)\
    Handler("syncLights", LXsTYPE_BOOLEAN, settings.sync_lights, true)\
    Handler("syncConstraints", LXsTYPE_BOOLEAN, settings.sync_constraints, true)\
    Handler("animationTS", LXsTYPE_FLOAT, settings.animation_time_scale, false)\
    Handler("animationSPS", LXsTYPE_FLOAT, settings.animation_sps, false)\
    Handler("keyframeReduction", LXsTYPE_BOOLEAN, settings.reduce_keyframes, false)\
    Handler("removeNamespace", LXsTYPE_BOOLEAN, settings.remove_namespace, true)


    void setup_args(CLxAttributeDesc &desc) override
    {
#define Handler(Name, Type, Member, Sync) desc.add(Name, Type);
        EachParams(Handler)
#undef Handler
        desc.add("dummy", LXsTYPE_BOOLEAN);
    }

    void interact() override
    {
        auto& settings = msmodoGetSettings();
#define Handler(Name, Type, Member, Sync) setArg(Name, Member);
        EachParams(Handler)
#undef Handler
    }

    void execute() override
    {
        auto& settings = msmodoGetSettings();
#define Handler(Name, Type, Member, Sync)\
        if(getArg(Name, Member) && settings.auto_sync && Sync)\
            msmodoGetInstance().sendScene(msmodoContext::SendScope::All, false);

        EachParams(Handler)
#undef Handler
    }

#undef EachParams
};


class msmodoCmdExport : public CLxCommand
{
public:
    void execute() override
    {
        msmodoGetInstance().sendScene(msmodoContext::SendScope::All, false);

        CLxUser_LogService lS;
        lS.DebugOut(LXi_DBLOG_NORMAL, msmodoCmdExportName " executed\n");
    }
};


class msmodoCmdImport : public CLxCommand
{
public:
    void execute() override
    {
        msmodoGetInstance().recvScene();

        CLxUser_LogService lS;
        lS.DebugOut(LXi_DBLOG_NORMAL, msmodoCmdImportName " executed\n");
    }
};

static CLxMeta_Command<msmodoCmdSettings> g_meta_settings(msmodoCmdSettingsName);
static CLxMeta_Command<msmodoCmdExport> g_meta_export(msmodoCmdExportName);
static CLxMeta_Command<msmodoCmdImport> g_meta_import(msmodoCmdImportName);

class msmodoMetaRoot : public CLxMetaRoot
{
    bool pre_init() override
    {
        g_meta_settings.set_type_UI();
        g_meta_settings.add_notifier(LXsNOTIFIER_SELECT, "");
        add(&g_meta_settings);
        add(&g_meta_export);
        add(&g_meta_import);
        return false;
    }
};
static msmodoMetaRoot g_mroot;



/*
class msmodoInstance :
    public CLxImpl_PackageInstance,
    public CLxImpl_SceneItemListener,
    public CLxImpl_ChannelUI
{
public:
    static void initialize()
    {
        auto srv = new CLxPolymorph<msmodoInstance>();
        srv->AddInterface(new CLxIfc_PackageInstance<msmodoInstance>());
        srv->AddInterface(new CLxIfc_SceneItemListener<msmodoInstance>());
        srv->AddInterface(new CLxIfc_ChannelUI<msmodoInstance>());
        lx::AddSpawner(msmodoInstanceName, srv);
    }

    msmodoInstance()
    {
        msLogInfo("msmodListener\n");
    }

    void sil_ChannelValue(const char *action, ILxUnknownID item, unsigned index) override
    {
        CLxUser_Item hit(item);

        CLxUser_LogService lS;
        lS.DebugOut(LXi_DBLOG_NORMAL, msmodoInstanceName " executed\n");
        // todo
    }
};


class msmodoPackage :
    public CLxImpl_Package,
    public CLxImpl_ChannelUI
{
public:
    static void initialize()
    {
        auto srv = new CLxPolymorph<msmodoPackage>();
        srv->AddInterface(new CLxIfc_Package<msmodoPackage>());
        //srv->AddInterface(new CLxIfc_StaticDesc<msmodoPackage>());
        srv->AddInterface(new CLxIfc_ChannelUI<msmodoPackage>());
        lx::AddSpawner(msmodoPackageName, srv);
    }

    CLxSpawner<msmodoInstance> m_inst_spawn;

    msmodoPackage()
        : m_inst_spawn(msmodoInstanceName)
    {
    }

    ~msmodoPackage()
    {
    }
};


void initialize(void)
{
    msmodoPackage::initialize();
    msmodoInstance::initialize();
}
*/
