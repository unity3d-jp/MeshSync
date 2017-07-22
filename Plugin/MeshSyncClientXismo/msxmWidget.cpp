#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncClientXismo.h"
using namespace mu;

#define msxmEnableQt

#ifdef msxmEnableQt
#include <QApplication>
#include <QWidget>
#include <QStackedWidget>
#include <QCheckbox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QIntValidator>
#include <QDoubleValidator>
#pragma comment(lib, "Qt5Core.lib")
#pragma comment(lib, "Qt5Gui.lib")
#pragma comment(lib, "Qt5Widgets.lib")


class XismoSyncSettingsWidget : public QWidget
{
//Q_OBJECT
using super = QWidget;
public:
    XismoSyncSettingsWidget(QWidget *parent = nullptr);

private:
    void onEditServer(const QString& v);
    void onEditPort(const QString& v);
    void onEditScaleFactor(const QString& v);
    void onToggleWelding(int v);
    void onToggleAutoSync(int v);
    void onClickManualSync(bool v);

private:
    QLineEdit *m_ed_server = nullptr;
    QLineEdit *m_ed_port = nullptr;
    QLineEdit *m_ed_scale_factor = nullptr;
    QCheckBox *m_ck_weld = nullptr;
    QCheckBox *m_ck_auto_sync = nullptr;
    QPushButton *m_bu_manual_sync = nullptr;
};

XismoSyncSettingsWidget::XismoSyncSettingsWidget(QWidget *parent)
    : super(parent)
{
    setWindowTitle("Unity Mesh Sync");
    setWindowFlags(Qt::Tool);
    //setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    //setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);

    msxmGetSettings().auto_sync = false;

    auto *layout = new QGridLayout();
    int iy = 0;

    layout->addWidget(new QLabel("Server:Port"), iy, 0);
    m_ed_server = new QLineEdit("localhost");
    layout->addWidget(m_ed_server, iy, 1);
    m_ed_port = new QLineEdit("8080");
    m_ed_port->setValidator(new QIntValidator(0, 65535, this));
    layout->addWidget(m_ed_port, iy++, 2);

    layout->addWidget(new QLabel("Scale Factor"), iy, 0);
    m_ed_scale_factor = new QLineEdit("100.0");
    m_ed_scale_factor->setValidator(new QDoubleValidator(0.0, 10000.0, 100, this));
    layout->addWidget(m_ed_scale_factor, iy++, 1);

    m_ck_weld = new QCheckBox("Weld Vertices");
    m_ck_weld->setCheckState(Qt::Checked);
    layout->addWidget(m_ck_weld, iy++, 0);

    m_ck_auto_sync = new QCheckBox("Auto Sync");
    layout->addWidget(m_ck_auto_sync, iy++, 0);

    m_bu_manual_sync = new QPushButton("Manual Sync");
    layout->addWidget(m_bu_manual_sync, iy++, 0);

    setLayout(layout);

    connect(m_ed_server, &QLineEdit::textEdited, this, &XismoSyncSettingsWidget::onEditServer);
    connect(m_ed_port, &QLineEdit::textEdited, this, &XismoSyncSettingsWidget::onEditPort);
    connect(m_ed_scale_factor, &QLineEdit::textEdited, this, &XismoSyncSettingsWidget::onEditScaleFactor);
    connect(m_ck_weld, &QCheckBox::stateChanged, this, &XismoSyncSettingsWidget::onToggleWelding);
    connect(m_ck_auto_sync, &QCheckBox::stateChanged, this, &XismoSyncSettingsWidget::onToggleAutoSync);
    connect(m_bu_manual_sync, &QPushButton::clicked, this, &XismoSyncSettingsWidget::onClickManualSync);
}

void XismoSyncSettingsWidget::onEditServer(const QString& v)
{
    auto mbs = v.toLatin1();
    msxmGetSettings().client_settings.server = mbs.data();
}

void XismoSyncSettingsWidget::onEditPort(const QString& v)
{
    auto& settings = msxmGetSettings();
    bool ok;
    uint16_t port = v.toUShort(&ok);
    if (ok) {
        settings.client_settings.port = port;
    }
}

void XismoSyncSettingsWidget::onEditScaleFactor(const QString& v)
{
    auto& settings = msxmGetSettings();
    bool ok;
    float scale = v.toFloat(&ok);
    if (ok && settings.scale_factor != scale) {
        settings.scale_factor = scale;
        msxmForceSetDirty();
    }
}

void XismoSyncSettingsWidget::onToggleWelding(int v)
{
    auto& settings = msxmGetSettings();
    if (settings.weld != (bool)v) {
        msxmGetSettings().weld = v;
        msxmForceSetDirty();
    }
}

void XismoSyncSettingsWidget::onToggleAutoSync(int v)
{
    msxmGetSettings().auto_sync = v;
}

void XismoSyncSettingsWidget::onClickManualSync(bool v)
{
    if (v) {
        msxmSend(true);
    }
}

static std::unique_ptr<XismoSyncSettingsWidget> g_widget;

void msxmInitializeWidget()
{
    if (!g_widget) {
        auto app = dynamic_cast<QApplication*>(QApplication::instance());
        if (app) {
            g_widget.reset(new XismoSyncSettingsWidget());
            g_widget->show();
        }
    }
}

#else // msxmEnableQt

void msxmInitializeWidget()
{
}

#endif // msxmEnableQt
