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
    Handler("bakeTransform", LXsTYPE_BOOLEAN, settings.bake_transform, true)\
    Handler("syncBlendShapes", LXsTYPE_BOOLEAN, settings.sync_blendshapes, true)\
    Handler("syncBones", LXsTYPE_BOOLEAN, settings.sync_bones, true)\
    Handler("syncTextures", LXsTYPE_BOOLEAN, settings.sync_textures, true)\
    Handler("syncCameras", LXsTYPE_BOOLEAN, settings.sync_cameras, true)\
    Handler("syncLights", LXsTYPE_BOOLEAN, settings.sync_lights, true)\
    Handler("syncMeshInstances", LXsTYPE_BOOLEAN, settings.sync_mesh_instances, true)\
    Handler("syncReplicators", LXsTYPE_BOOLEAN, settings.sync_replicators, true)\
    Handler("frameStep", LXsTYPE_FLOAT, settings.frame_step, false)


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
            msmodoGetContext().sendObjects(ObjectScope::All, true);

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
            static_assert((int)ExportTarget::Everything == 3, "SendTarget enum and uivalue mismatch");
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
        auto target = ExportTarget::Objects;
        cmd_read_arg("target", (int&)target);
        msmodoExport(target, ObjectScope::All);
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

class msmodoCmdCache : public CLxCommand
{
public:
    class ScopeSelector : public CLxCustomArgumentUI
    {
    public:
        CLxDynamicUIValue* uivalue(CLxCommand &cmd) override
        {
            auto *pop = new CLxUIValue();
            pop->popup_add("All");
            pop->popup_add("Selected");
            return pop;
        }
    };

    class FrameRangeSelector : public CLxCustomArgumentUI
    {
    public:
        CLxDynamicUIValue* uivalue(CLxCommand &cmd) override
        {
            auto *pop = new CLxUIValue();
            pop->popup_add("Current");
            pop->popup_add("All");
            pop->popup_add("Custom");
            return pop;
        }
    };

    class MaterialFrameRangeSelector : public CLxCustomArgumentUI
    {
    public:
        CLxDynamicUIValue* uivalue(CLxCommand &cmd) override
        {
            auto *pop = new CLxUIValue();
            pop->popup_add("None");
            pop->popup_add("One");
            pop->popup_add("All");
            return pop;
        }
    };

    void setup_args(CLxAttributeDesc &desc) override
    {
        desc.add("path", LXsTYPE_FILEPATH);

        desc.add("objectScope", LXsTYPE_INTEGER);
        desc.arg_set_custom(new ScopeSelector());

        desc.add("frameRange", LXsTYPE_INTEGER);
        desc.arg_set_custom(new FrameRangeSelector());
        desc.default_val(1);
        desc.add("frameBegin", LXsTYPE_INTEGER);
        desc.default_val(0);
        desc.add("frameEnd", LXsTYPE_INTEGER);
        desc.default_val(100);
        desc.add("frameStep", LXsTYPE_FLOAT);
        desc.default_val(1.0f);

        desc.add("materialFrameRange", LXsTYPE_INTEGER);
        desc.arg_set_custom(new MaterialFrameRangeSelector());
        desc.default_val(1);

        desc.add("zstdCompressionLevel", LXsTYPE_INTEGER);
        desc.default_val(3);

        desc.add("makeDoubleSided", LXsTYPE_BOOLEAN);
        desc.default_val(false);

        desc.add("bakeDeformers", LXsTYPE_BOOLEAN);
        desc.default_val(true);

        desc.add("bakeTransform", LXsTYPE_BOOLEAN);
        desc.default_val(true);

        desc.add("flattenHierarchy", LXsTYPE_BOOLEAN);
        desc.default_val(false);

        //desc.add("mergeMeshes", LXsTYPE_BOOLEAN);
        //desc.default_val(false);

        desc.add("stripNormals", LXsTYPE_BOOLEAN);
        desc.default_val(false);
        desc.add("stripTangents", LXsTYPE_BOOLEAN);
        desc.default_val(true);
    }

    inline bool readArg(const char *name, std::string& v)
    {
        return cmd_read_arg(name, v);
    }

    inline bool readArg(const char *name, int& v)
    {
        return cmd_read_arg(name, v);
    }

    inline bool readArg(const char *name, float& v)
    {
        double tmp;
        if (cmd_read_arg(name, tmp)) {
            v = (float)tmp;
            return true;
        }
        return false;
    }

    inline bool readArg(const char *name, bool& v)
    {
        int tmp;
        if (cmd_read_arg(name, tmp)) {
            v = (bool)tmp;
            return true;
        }
        return false;
    }

    void execute() override
    {
        auto settings = msmodoGetCacheSettings(); // copy

        readArg("path", settings.path);
        readArg("objectScope", (int&)settings.object_scope);
        readArg("frameRange", (int&)settings.frame_range);
        readArg("frameBegin", settings.frame_begin);
        readArg("frameEnd", settings.frame_end);
        readArg("frameStep", settings.frame_step);
        readArg("materialFrameRange", (int&)settings.material_frame_range);
        readArg("zstdCompressionLevel", settings.zstd_compression_level);
        readArg("makeDoubleSided", settings.make_double_sided);
        readArg("bakeDeformers", settings.bake_deformers);
        readArg("bakeTransform", settings.bake_transform);
        readArg("flattenHierarchy", settings.flatten_hierarchy);
        //readArg("mergeMeshes", settings.merge_meshes);
        readArg("stripNormals", settings.strip_normals);
        readArg("stripTangents", settings.strip_tangents);
        msmodoGetContext().exportCache(settings);
    }
};

static CLxMeta_Command<msmodoCmdSettings> g_meta_settings(msmodoCmdSettingsName);
static CLxMeta_Command<msmodoCmdExport> g_meta_export(msmodoCmdExportName);
static CLxMeta_Command<msmodoCmdImport> g_meta_import(msmodoCmdImportName);
static CLxMeta_Command<msmodoCmdCache> g_meta_cache(msmodoCmdCacheName);

class msmodoMetaRoot : public CLxMetaRoot
{
    bool pre_init() override
    {
        g_meta_settings.set_type_UI();
        g_meta_settings.add_notifier(LXsNOTIFIER_SELECT, "");
        add(&g_meta_settings);
        add(&g_meta_export);
        add(&g_meta_import);
        add(&g_meta_cache);
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
