#include "pch.h"
#include "MeshSyncClientModo.h"
#include "msmodoUtils.h"
#include "msmodoCommand.h"


class msmodoCmdSettings : public CLxCommand
{
public:
    void execute() override
    {
        CLxUser_LogService lS;
        lS.DebugOut(LXi_DBLOG_NORMAL, msmodoCmdSettingsName " executed\n");
    }
};


class msmodoCmdExport : public CLxCommand
{
public:
    void execute() override
    {
        CLxUser_LogService lS;
        lS.DebugOut(LXi_DBLOG_NORMAL, msmodoCmdExportName " executed\n");
    }
};


class msmodoCmdImport : public CLxCommand
{
public:
    void execute() override
    {
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

        g_meta_export.set_type_UI();
        g_meta_export.add_notifier(LXsNOTIFIER_SELECT, "");
        add(&g_meta_export);

        g_meta_import.set_type_UI();
        g_meta_import.add_notifier(LXsNOTIFIER_SELECT, "");
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
