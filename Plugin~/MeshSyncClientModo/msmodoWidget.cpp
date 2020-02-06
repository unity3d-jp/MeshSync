#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "msmodoContext.h"

//#define msmodoEnableQt

#ifdef msmodoEnableQt
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QWidget>
#include <QStackedWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QCloseEvent>
#include <QScrollArea>

#ifdef _WIN32
    #pragma comment(lib, "QtCore4.lib")
    #pragma comment(lib, "QtGui4.lib")
#endif


class msmodoSettingsWidget : public QWidget
{
using super = QWidget;
Q_OBJECT
public:
    msmodoSettingsWidget(QWidget *parent = nullptr);

public slots:
    void onEditServer(const QString& v);
    void onEditPort(const QString& v);
    void onEditScaleFactor(const QString& v);
    void onToggleSyncMeshes(int v);
    void onToggleSyncBones(int v);
    void onToggleSyncBlendshapes(int v);
    void onToggleDoubleSided(int v);
    void onToggleBakeDeformers(int v);
    void onToggleBakeTransform(int v);
    void onToggleSyncTextures(int v);
    void onToggleSyncMeshInstances(int v);
    void onToggleSyncReplicators(int v);
    void onToggleSyncCameras(int v);
    void onToggleSyncLights(int v);
    void onToggleAutoSync(int v);
    void onClickManualSync(bool v);
    void onClickExportCache(bool v);

    void onEditAnimationSPS(const QString& v);
    void onClickSyncAnimations(bool v);

private:
    //void resizeEvent(QResizeEvent *event) override;

    QWidget *m_widget_mesh = nullptr;
    QCheckBox *m_ck_bake_deformers = nullptr;
    QCheckBox *m_ck_bake_transform = nullptr;
    QCheckBox *m_ck_auto_sync = nullptr;
    QWidget *m_scroll_content = nullptr;
    QScrollArea *m_scroll_area = nullptr;
};


static inline QString to_qstring(int v)
{
    char buf[128];
    sprintf(buf, "%d", v);
    return buf;
}
static inline QString to_qstring(float v)
{
    char buf[128];
    sprintf(buf, "%.2f", v);
    return buf;
}
static inline QString to_qstring(const std::string& v)
{
    return v.c_str();
}
static inline Qt::CheckState to_qcheckstate(bool v)
{
    return v ? Qt::Checked : Qt::Unchecked;
}
static inline std::string to_stdstring(const QString& v)
{
    // note: on linux, QString::toStdString() returns incorrect result.
    // QByteArray::length() seems return wrong value. so we reimplement toStdString() with std::strlen()
    auto raw = v.toAscii();
    auto *data = raw.constData();
    return std::string(data, std::strlen(data));
}

msmodoSettingsWidget::msmodoSettingsWidget(QWidget *parent)
    : super(parent)
{
    auto& settings = msmodoGetSettings();

    // setup controls

    auto *layout = new QGridLayout();
    layout->setVerticalSpacing(2);
    layout->setAlignment(Qt::AlignTop);

    const int space = 8;
    int iy = 0;

    // server & scene settings
    {
        auto lb_server = new QLabel("Server");
        lb_server->setStyleSheet("font-weight: bold");
        layout->addWidget(lb_server, iy++, 0, 1, 3);

        layout->addWidget(new QLabel("Server:Port"), iy, 0);

        auto ed_server = new QLineEdit(to_qstring(settings.client_settings.server));
        layout->addWidget(ed_server, iy, 1);
        connect(ed_server, SIGNAL(textEdited(QString)), this, SLOT(onEditServer(QString)));

        auto ed_port = new QLineEdit(to_qstring((int)settings.client_settings.port));
        ed_port->setValidator(new QIntValidator(0, 65535, this));
        layout->addWidget(ed_port, iy++, 2);
        connect(ed_port, SIGNAL(textEdited(QString)), this, SLOT(onEditPort(QString)));

        auto lb_scene = new QLabel("Scene");
        lb_scene->setContentsMargins(0, space, 0, 0);
        lb_scene->setStyleSheet("font-weight: bold");
        layout->addWidget(lb_scene, iy++, 0, 1, 3);

        layout->addWidget(new QLabel("Scale Factor"), iy, 0);
        auto ed_scale_factor = new QLineEdit(to_qstring(settings.scale_factor));
        ed_scale_factor->setValidator(new QDoubleValidator(0.0, 10000.0, 100, this));
        layout->addWidget(ed_scale_factor, iy++, 1, 1, 2);
        connect(ed_scale_factor, SIGNAL(textEdited(QString)), this, SLOT(onEditScaleFactor(QString)));
    }

    // mesh
    {
        auto ck_meshes = new QCheckBox("Sync Meshes");
        ck_meshes->setCheckState(to_qcheckstate(settings.sync_meshes));
        layout->addWidget(ck_meshes, iy++, 0, 1, 3);
        connect(ck_meshes, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncMeshes(int)));

        int iy2 = 0;
        m_widget_mesh = new QWidget();
        auto *layout_mesh = new QGridLayout();
        layout_mesh->setVerticalSpacing(2);
        layout_mesh->setContentsMargins(10, 0, 0, 0);

        auto ck_double_sided = new QCheckBox("Make Double Sided");
        ck_double_sided->setCheckState(to_qcheckstate(settings.make_double_sided));
        layout_mesh->addWidget(ck_double_sided, iy2++, 0);
        connect(ck_double_sided, SIGNAL(stateChanged(int)), this, SLOT(onToggleDoubleSided(int)));

        m_ck_bake_deformers = new QCheckBox("Bake Deformers");
        m_ck_bake_deformers->setCheckState(to_qcheckstate(settings.bake_deformers));
        layout_mesh->addWidget(m_ck_bake_deformers, iy2++, 0);
        connect(m_ck_bake_deformers, SIGNAL(stateChanged(int)), this, SLOT(onToggleBakeDeformers(int)));

        m_ck_bake_transform = new QCheckBox("Bake Transform");
        m_ck_bake_transform->setCheckState(to_qcheckstate(settings.bake_transform));
        layout_mesh->addWidget(m_ck_bake_transform, iy2++, 0);
        connect(m_ck_bake_transform, SIGNAL(stateChanged(int)), this, SLOT(onToggleBakeTransform(int)));

        m_widget_mesh->setLayout(layout_mesh);
        m_widget_mesh->setShown(settings.sync_meshes);
        layout->addWidget(m_widget_mesh, iy++, 0, 1, 3, Qt::AlignTop);
    }

    // other components
    {
        auto ck_bones = new QCheckBox("Sync Joints");
        ck_bones->setCheckState(to_qcheckstate(settings.sync_bones));
        layout->addWidget(ck_bones, iy++, 0, 1, 3);
        connect(ck_bones, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncBones(int)));

        auto ck_blendshapes = new QCheckBox("Sync Morphs");
        ck_blendshapes->setCheckState(to_qcheckstate(settings.sync_blendshapes));
        layout->addWidget(ck_blendshapes, iy++, 0, 1, 3);
        connect(ck_blendshapes, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncBlendshapes(int)));

        auto ck_textures = new QCheckBox("Sync Textures");
        ck_textures->setCheckState(to_qcheckstate(settings.sync_textures));
        layout->addWidget(ck_textures, iy++, 0, 1, 3);
        connect(ck_textures, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncTextures(int)));

        auto ck_minst = new QCheckBox("Sync Mesh Instances");
        ck_minst->setCheckState(to_qcheckstate(settings.sync_mesh_instances));
        layout->addWidget(ck_minst, iy++, 0, 1, 3);
        connect(ck_minst, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncMeshInstances(int)));

        auto ck_replicators = new QCheckBox("Sync Replicators");
        ck_replicators->setCheckState(to_qcheckstate(settings.sync_replicators));
        layout->addWidget(ck_replicators, iy++, 0, 1, 3);
        connect(ck_replicators, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncReplicators(int)));

        auto ck_cameras = new QCheckBox("Sync Cameras");
        ck_cameras->setCheckState(to_qcheckstate(settings.sync_cameras));
        layout->addWidget(ck_cameras, iy++, 0, 1, 3);
        connect(ck_cameras, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncCameras(int)));

        auto ck_lights = new QCheckBox("Sync Lights");
        ck_lights->setCheckState(to_qcheckstate(settings.sync_lights));
        layout->addWidget(ck_lights, iy++, 0, 1, 3);
        connect(ck_lights, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncLights(int)));
    }

    {
        m_ck_auto_sync = new QCheckBox("Auto Sync");
        m_ck_auto_sync->setContentsMargins(0, space, 0, 0);
        m_ck_auto_sync->setCheckState(to_qcheckstate(settings.auto_sync));
        layout->addWidget(m_ck_auto_sync, iy++, 0, 1, 3);
        connect(m_ck_auto_sync, SIGNAL(stateChanged(int)), this, SLOT(onToggleAutoSync(int)));

        auto bu_manual_sync = new QPushButton("Manual Sync");
        layout->addWidget(bu_manual_sync, iy++, 0, 1, 3);
        connect(bu_manual_sync, SIGNAL(clicked(bool)), this, SLOT(onClickManualSync(bool)));
    }

    // animation
    {
        auto lb_animation = new QLabel("Animation");
        lb_animation->setContentsMargins(0, space, 0, 0);
        lb_animation->setStyleSheet("font-weight: bold");
        layout->addWidget(lb_animation, iy++, 0, 1, 3);

        layout->addWidget(new QLabel("Samples Per Second"), iy, 0);
        auto ed_anim_sps = new QLineEdit(to_qstring(settings.frame_step));
        ed_anim_sps->setValidator(new QDoubleValidator(0.01, 300.0, 100, this));
        layout->addWidget(ed_anim_sps, iy++, 1, 1, 2);
        connect(ed_anim_sps, SIGNAL(textEdited(QString)), this, SLOT(onEditAnimationSPS(QString)));

        auto bu_sync_animations = new QPushButton("Sync Animations");
        layout->addWidget(bu_sync_animations, iy++, 0, 1, 3);
        connect(bu_sync_animations, SIGNAL(clicked(bool)), this, SLOT(onClickSyncAnimations(bool)));
    }

    // cache
    {
        auto lb_cache = new QLabel("Cache");
        lb_cache->setContentsMargins(0, space, 0, 0);
        lb_cache->setStyleSheet("font-weight: bold");
        layout->addWidget(lb_cache, iy++, 0, 1, 3);

        auto bu_export_cache = new QPushButton("Export Cache");
        layout->addWidget(bu_export_cache, iy++, 0, 1, 3);
        connect(bu_export_cache, SIGNAL(clicked(bool)), this, SLOT(onClickExportCache(bool)));
    }

    {
        auto lb_version = new QLabel("Plugin Version: " msPluginVersionStr);
        lb_version->setContentsMargins(0, space, 0, 0);
        layout->addWidget(lb_version, iy++, 0, 1, 3);
    }

    setLayout(layout);

    //m_scroll_content = new QWidget(this);
    //m_scroll_content->setLayout(layout);

    //m_scroll_area = new QScrollArea();
    //m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //m_scroll_area->setWidgetResizable(true);
    //m_scroll_area->setWidget(m_scroll_content);
}


static bool msmodoSendObjects()
{
    return msmodoExport(ExportTarget::Objects, ObjectScope::All);
}

static bool msmodoSendAnimations()
{
    return msmodoExport(ExportTarget::Animations, ObjectScope::All);
}

void msmodoSettingsWidget::onEditServer(const QString& v)
{
    auto& settings = msmodoGetSettings();
    settings.client_settings.server = to_stdstring(v);
}

void msmodoSettingsWidget::onEditPort(const QString& v)
{
    auto& settings = msmodoGetSettings();
    bool ok;
    uint16_t val = v.toUShort(&ok);
    if (ok && val != 0) {
        settings.client_settings.port = val;
    }
}

void msmodoSettingsWidget::onEditScaleFactor(const QString& v)
{
    auto& settings = msmodoGetSettings();
    bool ok;
    float val = v.toFloat(&ok);
    if (ok && val != 0.0f && settings.scale_factor != val) {
        settings.scale_factor = val;
        if (settings.auto_sync)
            msmodoSendObjects();
    }
}

void msmodoSettingsWidget::onToggleSyncMeshes(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_meshes = v;
    m_widget_mesh->setShown(settings.sync_meshes);
    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleSyncBones(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_bones = v;
    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleSyncBlendshapes(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_blendshapes = v;
    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleDoubleSided(int v)
{
    auto& settings = msmodoGetSettings();
    settings.make_double_sided = v;
    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleBakeDeformers(int v)
{
    auto& settings = msmodoGetSettings();
    settings.bake_deformers = v;
    if (!settings.bake_deformers) {
        settings.bake_transform = false;
        m_ck_bake_transform->setCheckState(to_qcheckstate(settings.bake_transform));
    }

    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleBakeTransform(int v)
{
    auto& settings = msmodoGetSettings();
    settings.bake_transform = v;
    if (settings.bake_transform) {
        settings.bake_deformers = true;
        m_ck_bake_deformers->setCheckState(to_qcheckstate(settings.bake_deformers));
    }
    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleSyncTextures(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_textures = v;
}

void msmodoSettingsWidget::onToggleSyncMeshInstances(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_mesh_instances = v;
    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleSyncReplicators(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_replicators = v;
    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleSyncCameras(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_cameras = v;
    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleSyncLights(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_lights = v;
    if (settings.auto_sync)
        msmodoSendObjects();
}

void msmodoSettingsWidget::onToggleAutoSync(int v)
{
    auto& ctx = msmodoGetContext();
    auto& settings = msmodoGetSettings();
    if (v) {
        if (ctx.isServerAvailable()) {
            settings.auto_sync = true;
            msmodoSendObjects();
        }
        else {
            ctx.logError("MeshSync: Server not available. %s", ctx.getErrorMessage().c_str());
            m_ck_auto_sync->setCheckState(to_qcheckstate(false));
        }
    }
    else {
        settings.auto_sync = false;
    }
}

void msmodoSettingsWidget::onClickManualSync(bool v)
{
    msmodoSendObjects();
}

void msmodoSettingsWidget::onClickExportCache(bool v)
{
    CLxUser_CommandService svc_cmd;
    std::string path;
    {
        // open file dialog
        svc_cmd.ExecuteArgString(-1, LXiCTAG_NULL, "dialog.setup fileSave");
        svc_cmd.ExecuteArgString(-1, LXiCTAG_NULL, "dialog.title {Export Cache}");
        svc_cmd.ExecuteArgString(-1, LXiCTAG_NULL, "dialog.fileTypeCustom cache {Scene Cache} {*.sc} sc");
        svc_cmd.ExecuteArgString(-1, LXiCTAG_NULL, "dialog.open");

        CLxUser_Command cmd;
        CLxUser_ValueArray va;
        if (LXx_OK(svc_cmd.NewCommand(cmd, "dialog.result"))) {
            if (LXx_OK(svc_cmd.QueryIndex(cmd, 0, va))) {
                if (va.Count() > 0)
                    va.String(0, path);
            }
        }
    }
    if (!path.empty()) {
        char buf[1024];
        sprintf(buf, "unity.meshsync.cache path:{%s}", path.c_str());
        svc_cmd.ExecuteArgString(-1, LXiCTAG_NULL, buf);
    }
}

void msmodoSettingsWidget::onEditAnimationSPS(const QString& v)
{
    auto& settings = msmodoGetSettings();
    bool ok;
    float val = v.toFloat(&ok);
    if (ok && val != 0.0f && settings.frame_step != val) {
        settings.frame_step = val;
    }
}

void msmodoSettingsWidget::onClickSyncAnimations(bool v)
{
    msmodoSendAnimations();
}

//void msmodoSettingsWidget::resizeEvent(QResizeEvent *event)
//{
//    auto s = size();
//    m_scroll_content->resize(s);
//}


#include "msmodoWidget_moc.h"


void msmodoInitializeWidget(void *parent)
{
    auto parent_widget = static_cast<QWidget*>(parent);

    auto layout = new QVBoxLayout(parent_widget);
    layout->addWidget(new msmodoSettingsWidget());
    parent_widget->setLayout(layout);
}

#else

void msmodoInitializeWidget(void *parent)
{
}

#endif
