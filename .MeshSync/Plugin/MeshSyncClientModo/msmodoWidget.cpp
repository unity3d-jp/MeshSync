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
    void onToggleSyncTextures(int v);
    void onToggleDoubleSided(int v);
    void onToggleAutoSync(int v);
    void onClickManualSync(bool v);
    void onMenuAction(bool v);
    void closeEvent(QCloseEvent *event) override;

private:
    QAction *m_menu_item = nullptr;
};


msmodoSettingsWidget::msmodoSettingsWidget(QWidget *parent)
    : super(parent)
{
    setWindowTitle("Unity Mesh Sync");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    auto& settings = msmodoGetSettings();

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
    auto ed_scale_factor = new QLineEdit("1.0");
    ed_scale_factor->setValidator(new QDoubleValidator(0.0, 10000.0, 100, this));
    layout->addWidget(ed_scale_factor, iy++, 1, 1, 2);

    auto ck_double_sided = new QCheckBox("Double Sided");
    if (settings.make_double_sided)
        ck_double_sided->setCheckState(Qt::Checked);
    layout->addWidget(ck_double_sided, iy++, 0, 1, 3);

    auto ck_textures = new QCheckBox("Sync Textures");
    if (settings.sync_textures)
        ck_textures->setCheckState(Qt::Checked);
    layout->addWidget(ck_textures, iy++, 0, 1, 3);

    auto ck_auto_sync = new QCheckBox("Auto Sync");
    if (settings.auto_sync)
        ck_auto_sync->setCheckState(Qt::Checked);
    layout->addWidget(ck_auto_sync, iy++, 0, 1, 3);

    auto bu_manual_sync = new QPushButton("Manual Sync");
    layout->addWidget(bu_manual_sync, iy++, 0, 1, 3);

    layout->addWidget(new QLabel("Plugin Version: " msReleaseDateStr), iy++, 0, 1, 3);

    setLayout(layout);

    connect(ed_server, SIGNAL(textEdited(QString)), this, SLOT(onEditServer(QString)));
    connect(ed_port, SIGNAL(textEdited(QString)), this, SLOT(onEditPort(QString)));
    connect(ed_scale_factor, SIGNAL(textEdited(QString)), this, SLOT(onEditScaleFactor(QString)));
    connect(ck_double_sided, SIGNAL(stateChanged(int)), this, SLOT(onToggleDoubleSided(int)));
    connect(ck_textures, SIGNAL(stateChanged(int)), this, SLOT(onToggleSyncTextures(int)));
    connect(ck_auto_sync, SIGNAL(stateChanged(int)), this, SLOT(onToggleAutoSync(int)));
    connect(bu_manual_sync, SIGNAL(clicked(bool)), this, SLOT(onClickManualSync(bool)));


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
    uint16_t port = v.toUShort(&ok);
    if (ok) {
        settings.client_settings.port = port;
    }
}

void msmodoSettingsWidget::onEditScaleFactor(const QString& v)
{
    auto& settings = msmodoGetSettings();
    bool ok;
    float scale = v.toFloat(&ok);
    if (ok && settings.scale_factor != scale) {
        settings.scale_factor = scale;
        if (settings.auto_sync)
            msmodoGetInstance().sendScene(msmodoContext::SendScope::All, true);
    }
}

void msmodoSettingsWidget::onToggleSyncTextures(int v)
{
    auto& settings = msmodoGetSettings();
    settings.sync_textures = v;
}

void msmodoSettingsWidget::onToggleDoubleSided(int v)
{
    auto& settings = msmodoGetSettings();
    settings.make_double_sided = v;
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
