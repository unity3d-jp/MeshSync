#include "pch.h"
#include "msmodoUtils.h"
#include "msmodoCommand.h"
#include "msmodoContext.h"


class msmodoCmdSettings : public CLxCommand
{
public:
    bool getArg(const char *name, bool& dst)
    {
        int tmp;
        if (cmd_read_arg(name, tmp)) {
            bool old = dst;
            dst = (tmp != 0);
            return dst != old;
        }
        return false;
    }
    bool getArg(const char *name, int& dst)
    {
        int tmp;
        if (cmd_read_arg(name, tmp)) {
            int old = dst;
            dst = tmp;
            return dst != old;
        }
        return false;
    }
    bool getArg(const char *name, uint16_t& dst)
    {
        int tmp;
        if (cmd_read_arg(name, tmp)) {
            uint16_t old = dst;
            dst = (uint16_t)tmp;
            return dst != old;
        }
        return false;
    }
    bool getArg(const char *name, float& dst)
    {
        double tmp;
        if (cmd_read_arg(name, tmp)) {
            float old = dst;
            dst = (float)tmp;
            return dst != old;
        }
        return false;
    }
    bool getArg(const char *name, std::string& dst)
    {
        std::string tmp;
        if (cmd_read_arg(name, tmp)) {
            std::string old = dst;
            dst = tmp;
            return dst != old;
        }
        return false;
    }

#define EachParam(Handler)\
    Handler("serverAddress", LXsTYPE_STRING, settings.client_settings.server, false)\
    Handler("serverPort", LXsTYPE_INTEGER, settings.client_settings.port, false)\
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
    Handler("syncMeshInstances", LXsTYPE_BOOLEAN, settings.sync_mesh_instances, true)\
    Handler("syncReplicators", LXsTYPE_BOOLEAN, settings.sync_replicators, true)\
    Handler("animationTimeScale", LXsTYPE_FLOAT, settings.animation_time_scale, false)\
    Handler("animationSamplesPerSecond", LXsTYPE_FLOAT, settings.animation_sps, false)\
    Handler("keyframeReduction", LXsTYPE_BOOLEAN, settings.reduce_keyframes, false)\
    Handler("keepFlatCurves", LXsTYPE_BOOLEAN, settings.keep_flat_curves, false)


    void setup_args(CLxAttributeDesc &desc) override
    {
        auto& settings = msmodoGetSettings();
#define Handler(Name, Type, Member, Sync) desc.add(Name, Type); desc.default_val(Member);
        EachParam(Handler)
#undef Handler
    }

    void execute() override
    {
        auto& settings = msmodoGetSettings();
#define Handler(Name, Type, Member, Sync)\
        if(getArg(Name, Member) && settings.auto_sync && Sync)\
            msmodoGetContext().sendObjects(msmodoContext::SendScope::All, true);

        EachParam(Handler)
#undef Handler
    }

#undef EachParams
};


class msmodoCmdExport : public CLxCommand
{
public:
    class TargetSelector : public CLxCustomArgumentUI
    {
    public:
        CLxDynamicUIValue* uivalue(CLxCommand &cmd) override
        {
            auto *pop = new CLxUIValue();
            pop->popup_add("Objects");
            pop->popup_add("Materials");
            pop->popup_add("Animations");
            pop->popup_add("Everything");
            static_assert((int)msmodoContext::SendTarget::Everything == 3, "SendTarget enum and uivalue mismatch");
            return pop;
        }
    };

    void setup_args(CLxAttributeDesc &desc) override
    {
        desc.add("target", LXsTYPE_INTEGER);
        desc.arg_set_custom(new TargetSelector());
    }

    void execute() override
    {
        auto target = msmodoContext::SendTarget::Objects;
        cmd_read_arg("target", (int&)target);
        msmodoExport(target, msmodoContext::SendScope::All);
    }
};


class msmodoCmdImport : public CLxCommand
{
public:
    void execute() override
    {
        msmodoGetContext().recvObjects();
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


void msmodoInitializeWidget(void *parent);

class msmodoView : public CLxImpl_CustomView
{
public:
    LxResult customview_Init(ILxUnknownID pane)
    {
        CLxLoc_CustomPane p(pane);
        if (!p.test())
            return LXe_FAILED;

        void *parent = nullptr; // QWidget
        p.GetParent(&parent);
        if (!parent)
            return LXe_FAILED;

        msmodoInitializeWidget(parent);
        return LXe_OK;
    }
};


void initialize(void)
{
    {
        auto view = new CLxPolymorph<msmodoView>();
        view->AddInterface(new CLxIfc_CustomView<msmodoView>());
        lx::AddServer(msmodoViewName, view);
    }
}

void cleanup()
{
    msmodoContext::finalizeInstance();
}
