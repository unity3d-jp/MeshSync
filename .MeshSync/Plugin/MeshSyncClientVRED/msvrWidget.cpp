#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "msvrContext.h"

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
#pragma comment(lib, "Qt5Core.lib")
#pragma comment(lib, "Qt5Gui.lib")
#pragma comment(lib, "Qt5Widgets.lib")


class msvrSettingsWidget : public QWidget
{
using super = QWidget;
public:
    msvrSettingsWidget(QWidget *parent = nullptr);

private:
    void onEditServer(const QString& v);
    void onEditPort(const QString& v);
    void onEditScaleFactor(const QString& v);
    void onToggleSyncTextures(int v);
    void onToggleSyncCamera(int v);
    void onEditCameraPath(const QString& v);
    void onToggleSyncDelete(int v);
    void onToggleFlipU(int v);
    void onToggleFlipV(int v);
    void onToggleDoubleSided(int v);
    void onToggleAutoSync(int v);
    void onClickManualSync(bool v);
    void onMenuAction(bool v);
    void closeEvent(QCloseEvent *event) override;

    QAction *m_menu_item = nullptr;
};


static QMainWindow* FindMainWindow()
{
    auto widgets = qApp->topLevelWidgets();
    QMainWindow *ret = nullptr;
    for (auto w : widgets) {
        auto title = w->windowTitle().toStdString();
        if (auto mw = dynamic_cast<QMainWindow*>(w)) {
            if (mw->menuBar()->actions().size() > 8) {
                ret = mw;
            }
        }
    }
    return ret;
}


msvrSettingsWidget::msvrSettingsWidget(QWidget *parent)
    : super(parent)
{
    setWindowTitle("Unity Mesh Sync");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    auto &settings = msvrGetContext()->getSettings();

    // setup controls

    auto *layout = new QGridLayout();
    int iy = 0;

    layout->addWidget(new QLabel("Server:Port"), iy, 0);
    auto ed_server = new QLineEdit("localhost");
    layout->addWidget(ed_server, iy, 1);
    auto ed_port = new QLineEdit("8080");
    ed_port->setValidator(new QIntValidator(0, 65535, this));
    layout->addWidget(ed_port, iy++, 2);

    layout->addWidget(new QLabel("Scale Factor"), iy, 0);
    auto ed_scale_factor = new QLineEdit("100.0");
    ed_scale_factor->setValidator(new QDoubleValidator(0.0, 10000.0, 100, this));
    layout->addWidget(ed_scale_factor, iy++, 1, 1, 2);

    auto ck_flip_u = new QCheckBox("Flip U");
    if (settings.flip_u)
        ck_flip_u->setCheckState(Qt::Checked);
    layout->addWidget(ck_flip_u, iy++, 0, 1, 3);

    auto ck_flip_v = new QCheckBox("Flip V");
    if (settings.flip_v)
        ck_flip_v->setCheckState(Qt::Checked);
    layout->addWidget(ck_flip_v, iy++, 0, 1, 3);

    auto ck_double_sided = new QCheckBox("Double Sided");
    if (settings.make_double_sided)
        ck_double_sided->setCheckState(Qt::Checked);
    layout->addWidget(ck_double_sided, iy++, 0, 1, 3);

    auto ck_delete = new QCheckBox("Sync Delete");
    if (settings.sync_delete)
        ck_delete->setCheckState(Qt::Checked);
    layout->addWidget(ck_delete, iy++, 0, 1, 3);

    auto ck_textures = new QCheckBox("Sync Textures");
    if (settings.sync_textures)
        ck_textures->setCheckState(Qt::Checked);
    layout->addWidget(ck_textures, iy++, 0, 1, 3);

    auto ck_camera = new QCheckBox("Sync Camera");
    if (settings.sync_camera)
        ck_camera->setCheckState(Qt::Checked);
    layout->addWidget(ck_camera, iy++, 0, 1, 3);

    layout->addWidget(new QLabel("Camera Path"), iy, 0);
    auto ed_camera_path = new QLineEdit(settings.camera_path.c_str());
    layout->addWidget(ed_camera_path, iy++, 1);

    auto ck_auto_sync = new QCheckBox("Auto Sync");
    if (settings.auto_sync)
        ck_auto_sync->setCheckState(Qt::Checked);
    layout->addWidget(ck_auto_sync, iy++, 0, 1, 3);

    auto bu_manual_sync = new QPushButton("Manual Sync");
    layout->addWidget(bu_manual_sync, iy++, 0, 1, 3);

    layout->addWidget(new QLabel("Plugin Version: " msPluginVersionStr), iy++, 0, 1, 3);

    setLayout(layout);

    connect(ed_server, &QLineEdit::textEdited, this, &msvrSettingsWidget::onEditServer);
    connect(ed_port, &QLineEdit::textEdited, this, &msvrSettingsWidget::onEditPort);
    connect(ed_scale_factor, &QLineEdit::textEdited, this, &msvrSettingsWidget::onEditScaleFactor);
    connect(ck_flip_u, &QCheckBox::stateChanged, this, &msvrSettingsWidget::onToggleFlipU);
    connect(ck_flip_v, &QCheckBox::stateChanged, this, &msvrSettingsWidget::onToggleFlipV);
    connect(ck_double_sided, &QCheckBox::stateChanged, this, &msvrSettingsWidget::onToggleDoubleSided);
    connect(ck_delete, &QCheckBox::stateChanged, this, &msvrSettingsWidget::onToggleSyncDelete);
    connect(ck_textures, &QCheckBox::stateChanged, this, &msvrSettingsWidget::onToggleSyncTextures);
    connect(ck_camera, &QCheckBox::stateChanged, this, &msvrSettingsWidget::onToggleSyncCamera);
    connect(ed_camera_path, &QLineEdit::textEdited, this, &msvrSettingsWidget::onEditCameraPath);
    connect(ck_auto_sync, &QCheckBox::stateChanged, this, &msvrSettingsWidget::onToggleAutoSync);
    connect(bu_manual_sync, &QPushButton::clicked, this, &msvrSettingsWidget::onClickManualSync);


    // try to add menu item (Window -> Unity Mesh Sync)

    auto actions = FindMainWindow()->menuBar()->actions();
    if (actions.size() > 8) {
        auto *act_widgets = actions[8]; // "Window" menu

        m_menu_item = new QAction("Unity Mesh Sync", nullptr);
        m_menu_item->setCheckable(true);
        m_menu_item->setChecked(true);
        connect(m_menu_item, &QAction::triggered, this, &msvrSettingsWidget::onMenuAction);

        auto widget_menu = act_widgets->menu();
        widget_menu->addSeparator();
        widget_menu->addAction(m_menu_item);
    }
}

void msvrSettingsWidget::onEditServer(const QString& v)
{
    auto ctx = msvrGetContext();
    ctx->getSettings().client_settings.server = v.toStdString();
}

void msvrSettingsWidget::onEditPort(const QString& v)
{
    auto ctx = msvrGetContext();
    auto& settings = ctx->getSettings();
    bool ok;
    uint16_t port = v.toUShort(&ok);
    if (ok) {
        settings.client_settings.port = port;
    }
}

void msvrSettingsWidget::onEditScaleFactor(const QString& v)
{
    auto ctx = msvrGetContext();
    auto& settings = ctx->getSettings();
    bool ok;
    float scale = v.toFloat(&ok);
    if (ok && settings.scale_factor != scale) {
        settings.scale_factor = scale;
        if (settings.auto_sync)
            ctx->send(true);
    }
}

void msvrSettingsWidget::onToggleSyncTextures(int v)
{
    auto ctx = msvrGetContext();
    auto& settings = ctx->getSettings();
    settings.sync_textures = v;
}

void msvrSettingsWidget::onToggleSyncCamera(int v)
{
    auto ctx = msvrGetContext();
    auto& settings = ctx->getSettings();
    settings.sync_camera = v;
    if (settings.auto_sync)
        ctx->send(false);
}

void msvrSettingsWidget::onEditCameraPath(const QString & v)
{
    auto ctx = msvrGetContext();
    ctx->getSettings().camera_path = v.toStdString();
}

void msvrSettingsWidget::onToggleSyncDelete(int v)
{
    auto ctx = msvrGetContext();
    auto& settings = ctx->getSettings();
    settings.sync_delete = v;
}

void msvrSettingsWidget::onToggleFlipU(int v)
{
    auto ctx = msvrGetContext();
    auto& settings = msvrGetContext()->getSettings();
    ctx->flipU(v);
    if (settings.auto_sync)
        ctx->send(true);
}

void msvrSettingsWidget::onToggleFlipV(int v)
{
    auto ctx = msvrGetContext();
    auto& settings = msvrGetContext()->getSettings();
    ctx->flipV(v);
    if (settings.auto_sync)
        ctx->send(true);
}

void msvrSettingsWidget::onToggleDoubleSided(int v)
{
    auto ctx = msvrGetContext();
    auto& settings = msvrGetContext()->getSettings();
    ctx->makeDoubleSided(v);
    if (settings.auto_sync)
        ctx->send(true);
}

void msvrSettingsWidget::onToggleAutoSync(int v)
{
    auto ctx = msvrGetContext();
    auto& settings = ctx->getSettings();
    settings.auto_sync = v;
    if (v)
        ctx->send(true);
}

void msvrSettingsWidget::onClickManualSync(bool v)
{
    auto ctx = msvrGetContext();
    ctx->send(true);
}

void msvrSettingsWidget::onMenuAction(bool v)
{
    if (v)
        show();
    else
        hide();
}

void msvrSettingsWidget::closeEvent(QCloseEvent * e)
{
    if (m_menu_item) {
        m_menu_item->setChecked(false);
    }
}

static msvrSettingsWidget *g_widget = nullptr;

void msvrInitializeWidget()
{
    if (!g_widget) {
        if (auto mainwindow = FindMainWindow()) {
            g_widget = new msvrSettingsWidget();
            g_widget->show();
        }
    }
}

#else

void msvrInitializeWidget()
{
}

#endif
