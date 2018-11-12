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


class XismoSyncSettingsWidget : public QWidget
{
using super = QWidget;
public:
    XismoSyncSettingsWidget(QWidget *parent = nullptr);

private:
    void onEditServer(const QString& v);
    void onEditPort(const QString& v);
    void onEditScaleFactor(const QString& v);
    void onToggleWelding(int v);
    void onToggleSyncCamera(int v);
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


XismoSyncSettingsWidget::XismoSyncSettingsWidget(QWidget *parent)
    : super(parent)
{
    setWindowTitle("Unity Mesh Sync");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    auto ctx = msxmGetContext();
    ctx->getSettings().auto_sync = false;


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

    //auto ck_weld = new QCheckBox("Weld Vertices");
    //ck_weld->setCheckState(Qt::Checked);
    //layout->addWidget(ck_weld, iy++, 0, 1, 3);

    auto ck_delete = new QCheckBox("Sync Delete / Hide");
    ck_delete->setCheckState(Qt::Checked);
    layout->addWidget(ck_delete, iy++, 0, 1, 3);

    auto ck_camera = new QCheckBox("Sync Camera");
    layout->addWidget(ck_camera, iy++, 0, 1, 3);

    auto ck_auto_sync = new QCheckBox("Auto Sync");
    layout->addWidget(ck_auto_sync, iy++, 0, 1, 3);

    auto bu_manual_sync = new QPushButton("Manual Sync");
    layout->addWidget(bu_manual_sync, iy++, 0, 1, 3);

    setLayout(layout);

    connect(ed_server, &QLineEdit::textEdited, this, &XismoSyncSettingsWidget::onEditServer);
    connect(ed_port, &QLineEdit::textEdited, this, &XismoSyncSettingsWidget::onEditPort);
    connect(ed_scale_factor, &QLineEdit::textEdited, this, &XismoSyncSettingsWidget::onEditScaleFactor);
    //connect(ck_weld, &QCheckBox::stateChanged, this, &XismoSyncSettingsWidget::onToggleWelding);
    connect(ck_camera, &QCheckBox::stateChanged, this, &XismoSyncSettingsWidget::onToggleSyncCamera);
    connect(ck_delete, &QCheckBox::stateChanged, this, &XismoSyncSettingsWidget::onToggleSyncDelete);
    connect(ck_auto_sync, &QCheckBox::stateChanged, this, &XismoSyncSettingsWidget::onToggleAutoSync);
    connect(bu_manual_sync, &QPushButton::clicked, this, &XismoSyncSettingsWidget::onClickManualSync);


    // try to add menu item (Widget -> Unity Mesh Sync)

    auto actions = FindMainWindow()->menuBar()->actions();
    if (actions.size() > 9) {
        auto *act_widgets = actions[9]; // "Widgets" menu

        m_menu_item = new QAction("Unity Mesh Sync", nullptr);
        m_menu_item->setCheckable(true);
        m_menu_item->setChecked(true);
        connect(m_menu_item, &QAction::triggered, this, &XismoSyncSettingsWidget::onMenuAction);

        auto widget_menu = act_widgets->menu();
        widget_menu->addSeparator();
        widget_menu->addAction(m_menu_item);
    }
}

void XismoSyncSettingsWidget::onEditServer(const QString& v)
{
    auto ctx = msxmGetContext();
    ctx->getSettings().client_settings.server = v.toStdString();
}

void XismoSyncSettingsWidget::onEditPort(const QString& v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    bool ok;
    uint16_t port = v.toUShort(&ok);
    if (ok) {
        settings.client_settings.port = port;
    }
}

void XismoSyncSettingsWidget::onEditScaleFactor(const QString& v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    bool ok;
    float scale = v.toFloat(&ok);
    if (ok && settings.scale_factor != scale) {
        settings.scale_factor = scale;
        ctx->send(true);
    }
}

void XismoSyncSettingsWidget::onToggleWelding(int v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    if (settings.weld_vertices != (bool)v) {
        settings.weld_vertices = v;
        ctx->send(true);
    }
}

void XismoSyncSettingsWidget::onToggleSyncCamera(int v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    if (settings.sync_camera != (bool)v) {
        settings.sync_camera = v;
        ctx->send(true);
    }
}

void XismoSyncSettingsWidget::onToggleSyncDelete(int v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    settings.sync_delete = v;
}

void XismoSyncSettingsWidget::onToggleAutoSync(int v)
{
    auto ctx = msxmGetContext();
    auto& settings = ctx->getSettings();
    settings.auto_sync = v;
    if (v) {
        ctx->send(true);
    }
}

void XismoSyncSettingsWidget::onClickManualSync(bool v)
{
    auto ctx = msxmGetContext();
    ctx->send(true);
}

void XismoSyncSettingsWidget::onMenuAction(bool v)
{
    if (v)
        show();
    else
        hide();
}

void XismoSyncSettingsWidget::closeEvent(QCloseEvent * e)
{
    if (m_menu_item) {
        m_menu_item->setChecked(false);
    }
}

static std::unique_ptr<XismoSyncSettingsWidget> g_widget;


void msxmInitializeWidget()
{
    if (!g_widget) {
        g_widget.reset(new XismoSyncSettingsWidget());
        g_widget->show();
    }
}

#else // msxmEnableQt

void msxmInitializeWidget()
{
}

#endif // msxmEnableQt
