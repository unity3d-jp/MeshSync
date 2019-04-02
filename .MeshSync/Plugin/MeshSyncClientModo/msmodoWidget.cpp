#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "msmodoContext.h"

#define msvrEnableQt

#ifdef msvrEnableQt
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QWidget>
#include <QStackedWidget>
#include <QCheckbox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QCloseEvent>

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
    void onToggleBones(int v);
    void onToggleBlendshapes(int v);
    void onToggleBakeDeformers(int v);
    void onToggleDoubleSided(int v);
    void onToggleSyncTextures(int v);
    void onToggleSyncMeshInstances(int v);
    void onToggleSyncCameras(int v);
    void onToggleSyncLights(int v);
    void onToggleAutoSync(int v);
    void onClickManualSync(bool v);
    void onEditAnimationTimeScale(const QString& v);
    void onEditAnimationSPS(const QString& v);
    void onClickSyncAnimations(bool v);
    void onMenuAction(bool v);
    void closeEvent(QCloseEvent *event) override;

private:
    QAction *m_menu_item = nullptr;
};


inline QString to_qstring(int v)
{
    char buf[128];
    sprintf(buf, "%d", v);
    return buf;
}
inline QString to_qstring(float v)
{
    char buf[128];
    sprintf(buf, "%.2f", v);
    return buf;
}
inline QString to_qstring(const std::string& v)
{
    return v.c_str();
}

msmodoSettingsWidget::msmodoSettingsWidget(QWidget *parent)
    : super(parent)
{
    setWindowTitle("Unity Mesh Sync");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    auto& settings = msmodoGetSettings();

    // setup controls

    auto *layout = new QGridLayout();
    int iy = 0;

    // server & scene settings
    layout->addWidget(new QLabel("Server:Port"), iy, 0);
    auto ed_server = new QLineEdit(to_qstring(settings.client_settings.server));
    layout->addWidget(ed_server, iy, 1);
    auto ed_port = new QLineEdit(to_qstring((int)settings.client_settings.port));
    ed_port->setValidator(new QIntValidator(0, 65535, this));
    layout->addWidget(ed_port, iy++, 2);

    layout->addWidget(new QLabel("Scale Factor"), iy, 0);
    auto ed_scale_factor = new QLineEdit(to_qstring(settings.scale_factor));
    ed_scale_factor->setValidator(new QDoubleValidator(0.0, 10000.0, 100, this));
    layout->addWidget(ed_scale_factor, iy++, 1, 1, 2);

    // sync component
    auto ck_bones= new QCheckBox("Sync Joints");
    if (settings.sync_bones)
        ck_bones->setCheckState(Qt::Checked);
    layout->addWidget(ck_bones, iy++, 0, 1, 3);

    auto ck_blendshapes = new QCheckBox("Sync Morphs");
    if (settings.sync_blendshapes)
        ck_blendshapes->setCheckState(Qt::Checked);
    layout->addWidget(ck_blendshapes, iy++, 0, 1, 3);

    auto ck_bake_deformers = new QCheckBox("Bake Deformers");
    if (settings.bake_deformers)
        ck_bake_deformers->setCheckState(Qt::Checked);
    layout->addWidget(ck_bake_deformers, iy++, 0, 1, 3);

    auto ck_double_sided = new QCheckBox("Make Double Sided");
    if (settings.make_double_sided)
        ck_double_sided->setCheckState(Qt::Checked);
    layout->addWidget(ck_double_sided, iy++, 0, 1, 3);

    auto ck_textures = new QCheckBox("Sync Textures");
    if (settings.sync_textures)
        ck_textures->setCheckState(Qt::Checked);
    layout->addWidget(ck_textures, iy++, 0, 1, 3);

    auto ck_minst = new QCheckBox("Sync Mesh Instances");
    if (settings.sync_mesh_instances)
        ck_minst->setCheckState(Qt::Checked);
    layout->addWidget(ck_minst, iy++, 0, 1, 3);

    auto ck_cameras = new QCheckBox("Sync Cameras");
    if (settings.sync_cameras)
        ck_cameras->setCheckState(Qt::Checked);
    layout->addWidget(ck_cameras, iy++, 0, 1, 3);

    auto ck_lights = new QCheckBox("Sync Lights");
    if (settings.sync_lights)
        ck_lights->setCheckState(Qt::Checked);
    layout->addWidget(ck_lights, iy++, 0, 1, 3);

    auto ck_auto_sync = new QCheckBox("Auto Sync");
    if (settings.auto_sync)
        ck_auto_sync->setCheckState(Qt::Checked);
    layout->addWidget(ck_auto_sync, iy++, 0, 1, 3);

    auto bu_manual_sync = new QPushButton("Manual Sync");
    layout->addWidget(bu_manual_sync, iy++, 0, 1, 3);

    // animation

    layout->addWidget(new QLabel("Animation Time Scale"), iy, 0);
    auto ed_anim_timescale = new QLineEdit(to_qstring(settings.animation_time_scale));
    ed_anim_timescale->setValidator(new QDoubleValidator(0.01, 10000.0, 100, this));
    layout->addWidget(ed_anim_timescale, iy++, 1, 1, 2);

    layout->addWidget(new QLabel("Samples Per Second"), iy, 0);
    auto ed_anim_sps = new QLineEdit(to_qstring(settings.animation_sps));
    ed_anim_sps->setValidator(new QDoubleValidator(0.01, 300.0, 100, this));
    layout->addWidget(ed_anim_sps, iy++, 1, 1, 2);

    auto bu_sync_animations = new QPushButton("Sync Animations");
    layout->addWidget(bu_sync_animations, iy++, 0, 1, 3);

    layout->addWidget(new QLabel("Plugin Version: " msReleaseDateStr), iy++, 0, 1, 3);

    setLayout(layout);

    connect(ed_server, SIGNAL(textEdited(QString)), this, SLOT(onEditServer(QString)));
    connect(ed_port, SIGNAL(textEdited(QString)), this, SLOT(onEditPort(QString)));
    connect(ed_scale_factor, SIGNAL(textEdited(QString)), this, SLOT(onEditScaleFactor(QString)));

    connect(ck_bones, SIGNAL(stateChanged(int)), this, SLOT(onToggleBones(int)));
    connect(ck_blendshapes, SIGNAL(stateChanged(int)), this, SLOT(onToggleBlendshapes(int)));
    connect(ck_bake_deformers, SIGNAL(stateChanged(int)), this, SLOT(onToggleBakeDeformers(int)));
    connect(ck_double_sided, SIGNAL(stateChanged(int)), this, SLOT(onToggleDoubleSided(int)));
    connect(ck_textures, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncTextures(int)));
    connect(ck_minst, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncMeshInstances(int)));
    connect(ck_cameras, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncCameras(int)));
    connect(ck_lights, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncLights(int)));
    connect(ck_auto_sync, SIGNAL(stateChanged(int)), this, SLOT(onToggleAutoSync(int)));
    connect(bu_manual_sync, SIGNAL(clicked(bool)), this, SLOT(onClickManualSync(bool)));

    connect(ed_anim_timescale, SIGNAL(textEdited(QString)), this, SLOT(onEditAnimationTimeScale(QString)));
    connect(ed_anim_sps, SIGNAL(textEdited(QString)), this, SLOT(onEditAnimationSPS(QString)));
    connect(bu_sync_animations, SIGNAL(clicked(bool)), this, SLOT(onClickSyncAnimations(bool)));


    //// try to add menu item (Window -> Unity Mesh Sync)

    //auto actions = FindMainWindow()->menuBar()->actions();
    //if (actions.size() > 8) {
    //    auto *act_widgets = actions[8]; // "Window" menu

    //    m_menu_item = new QAction("Unity Mesh Sync", nullptr);
    //    m_menu_item->setCheckable(true);
    //    m_menu_item->setChecked(true);
    //    connect(m_menu_item, &QAction::triggered, this, &msmodoSettingsWidget::onMenuAction);

    //    auto widget_menu = act_widgets->menu();
    //    widget_menu->addSeparator();
    //    widget_menu->addAction(m_menu_item);
    //}
}

void msmodoSettingsWidget::onEditServer(const QString& v)
{
    auto& settings = msmodoGetSettings();
    settings.client_settings.server = v.toStdString();
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
            msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
    }
}

void msmodoSettingsWidget::onToggleBones(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_bones = v;
    if (settings.auto_sync)
        msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
}

void msmodoSettingsWidget::onToggleBlendshapes(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_blendshapes = v;
    if (settings.auto_sync)
        msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
}

void msmodoSettingsWidget::onToggleBakeDeformers(int v)
{
    auto& settings = msmodoGetSettings();
    settings.bake_deformers = v;
    if (settings.auto_sync)
        msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
}

void msmodoSettingsWidget::onToggleDoubleSided(int v)
{
    auto& settings = msmodoGetSettings();
    settings.make_double_sided = v;
    if (settings.auto_sync)
        msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
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
        msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
}

void msmodoSettingsWidget::onToggleSyncCameras(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_cameras = v;
    if (settings.auto_sync)
        msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
}

void msmodoSettingsWidget::onToggleSyncLights(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_lights = v;
    if (settings.auto_sync)
        msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
}

void msmodoSettingsWidget::onToggleAutoSync(int v)
{
    auto& settings = msmodoGetSettings();
    settings.auto_sync = v;
    if (v)
        msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
}

void msmodoSettingsWidget::onClickManualSync(bool v)
{
    msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
}


void msmodoSettingsWidget::onEditAnimationTimeScale(const QString& v)
{
    auto& settings = msmodoGetSettings();
    bool ok;
    float val = v.toFloat(&ok);
    if (ok && val != 0.0f && settings.animation_time_scale != val) {
        settings.animation_time_scale = val;
    }
}

void msmodoSettingsWidget::onEditAnimationSPS(const QString& v)
{
    auto& settings = msmodoGetSettings();
    bool ok;
    float val = v.toFloat(&ok);
    if (ok && val != 0.0f && settings.animation_sps != val) {
        settings.animation_sps = val;
        if (settings.auto_sync)
            msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
    }
}

void msmodoSettingsWidget::onClickSyncAnimations(bool v)
{
    msmodoGetInstance().sendAnimations(msmodoContext::SendScope::All);
}



void msmodoSettingsWidget::onMenuAction(bool v)
{
    if (v)
        show();
    else
        hide();
}

void msmodoSettingsWidget::closeEvent(QCloseEvent * e)
{
    if (m_menu_item) {
        m_menu_item->setChecked(false);
    }
}

static msmodoSettingsWidget *g_widget = nullptr;

void msmodoInitializeWidget()
{
    if (!g_widget) {
        g_widget = new msmodoSettingsWidget();
        g_widget->show();
    }
}

#include "msmodoWidget_moc.h"

#else

void msmodoInitializeWidget()
{
}

#endif
