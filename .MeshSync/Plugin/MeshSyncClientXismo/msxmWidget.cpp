#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "msxmContext.h"

#define msxmEnableQt

#ifdef msxmEnableQt
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


class msxmSettingsWidget : public QWidget
{
using super = QWidget;
public:
    msxmSettingsWidget(QWidget *parent = nullptr);

private:
    void onEditServer(const QString& v);
    void onEditPort(const QString& v);
    void onEditScaleFactor(const QString& v);
    void onToggleDoubleSided(int v);
    void onToggleSyncTextures(int v);
    void onToggleSyncCamera(int v);
    void onEditCameraPath(const QString& v);
    void onToggleSyncDelete(int v);
    void onToggleAutoSync(int v);
    void onClickManualSync(bool v);
    void onMenuAction(bool v);
    void closeEvent(QCloseEvent *event) override;

    QAction *m_menu_item = nullptr;
};


static QMainWindow* FindMainWindow()
{
    auto widgets = qApp->topLevelWidgets();
    for (auto w : widgets) {
        auto mw = dynamic_cast<QMainWindow*>(w);
        if (mw) {
            return mw;
        }
    }
    return nullptr;
}


msxmSettingsWidget::msxmSettingsWidget(QWidget *parent)
    : super(parent)
{
    setWindowTitle("Unity Mesh Sync");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    auto &settings = msxmGetContext()->getSettings();

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

    connect(ed_server, &QLineEdit::textEdited, this, &msxmSettingsWidget::onEditServer);
    connect(ed_port, &QLineEdit::textEdited, this, &msxmSettingsWidget::onEditPort);
    connect(ed_scale_factor, &QLineEdit::textEdited, this, &msxmSettingsWidget::onEditScaleFactor);
    connect(ck_double_sided, &QCheckBox::stateChanged, this, &msxmSettingsWidget::onToggleDoubleSided);
    connect(ck_textures, &QCheckBox::stateChanged, this, &msxmSettingsWidget::onToggleSyncTextures);
    connect(ck_camera, &QCheckBox::stateChanged, this, &msxmSettingsWidget::onToggleSyncCamera);
    connect(ed_camera_path, &QLineEdit::textEdited, this, &msxmSettingsWidget::onEditCameraPath);
    connect(ck_delete, &QCheckBox::stateChanged, this, &msxmSettingsWidget::onToggleSyncDelete);
    connect(ck_auto_sync, &QCheckBox::stateChanged, this, &msxmSettingsWidget::onToggleAutoSync);
    connect(bu_manual_sync, &QPushButton::clicked, this, &msxmSettingsWidget::onClickManualSync);


    // try to add menu item (Widget -> Unity Mesh Sync)

    auto actions = FindMainWindow()->menuBar()->actions();
    if (actions.size() > 9) {
        auto *act_widgets = actions[9]; // "Widgets" menu

        m_menu_item = new QAction("Unity Mesh Sync", nullptr);
        m_menu_item->setCheckable(true);
        m_menu_item->setChecked(true);
        connect(m_menu_item, &QAction::triggered, this, &msxmSettingsWidget::onMenuAction);

        auto widget_menu = act_widgets->menu();
        widget_menu->addSeparator();
        widget_menu->addAction(m_menu_item);
    }
}

void msxmSettingsWidget::onEditServer(const QString& v)
{
    auto ctx = msxmGetContext();
    ctx->getSettings().client_settings.server = v.toStdString();
}

void msxmSettingsWidget::onEditPort(const QString& v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    bool ok;
    uint16_t port = v.toUShort(&ok);
    if (ok) {
        settings.client_settings.port = port;
    }
}

void msxmSettingsWidget::onEditScaleFactor(const QString& v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    bool ok;
    float scale = v.toFloat(&ok);
    if (ok && settings.scale_factor != scale) {
        settings.scale_factor = scale;
        if (settings.auto_sync)
            ctx->send(true);
    }
}

void msxmSettingsWidget::onToggleDoubleSided(int v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    settings.make_double_sided = v;
    if (settings.auto_sync)
        ctx->send(true);
}

void msxmSettingsWidget::onToggleSyncTextures(int v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    settings.sync_textures = v;
}

void msxmSettingsWidget::onToggleSyncCamera(int v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    settings.sync_camera = v;
    if (settings.auto_sync)
        ctx->send(false);
}

void msxmSettingsWidget::onEditCameraPath(const QString & v)
{
    auto ctx = msxmGetContext();
    ctx->getSettings().camera_path = v.toStdString();
}

void msxmSettingsWidget::onToggleSyncDelete(int v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    settings.sync_delete = v;
}

void msxmSettingsWidget::onToggleAutoSync(int v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    settings.auto_sync = v;
    if (v)
        ctx->send(true);
}

void msxmSettingsWidget::onClickManualSync(bool v)
{
    auto ctx = msxmGetContext();
    ctx->send(true);
}

void msxmSettingsWidget::onMenuAction(bool v)
{
    if (v)
        show();
    else
        hide();
}

void msxmSettingsWidget::closeEvent(QCloseEvent * e)
{
    if (m_menu_item) {
        m_menu_item->setChecked(false);
    }
}

static msxmSettingsWidget *g_widget = nullptr;

void msxmInitializeWidget()
{
    if (!g_widget) {
        g_widget = new msxmSettingsWidget();
        g_widget->show();
    }
}

#else // msxmEnableQt

void msxmInitializeWidget()
{
}

#endif // msxmEnableQt
