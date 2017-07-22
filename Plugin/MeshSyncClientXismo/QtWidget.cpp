#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncClientXismo.h"
using namespace mu;

#define EnableQt
#ifdef EnableQt
#include <QApplication>
#include <QWidget>
#include <QStackedWidget>
#include <QCheckbox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
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
    QLineEdit *m_ed_server = nullptr;
    QLineEdit *m_ed_port = nullptr;
    QLineEdit *m_ed_scale_factor = nullptr;
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

    auto *layout = new QGridLayout();
    int iy = 0;


    layout->addWidget(new QLabel("Server:Port"), iy, 0);
    m_ed_server = new QLineEdit("localhost");
    layout->addWidget(m_ed_server, iy, 1);
    m_ed_port = new QLineEdit("8080");
    layout->addWidget(m_ed_port, iy++, 2);

    layout->addWidget(new QLabel("Scale Factor"), iy, 0);
    m_ed_scale_factor = new QLineEdit("100.0");
    layout->addWidget(m_ed_scale_factor, iy++, 1);

    m_ck_auto_sync = new QCheckBox("Auto Sync");
    layout->addWidget(m_ck_auto_sync, iy++, 0);

    m_bu_manual_sync = new QPushButton("Manual Sync");
    layout->addWidget(m_bu_manual_sync, iy++, 0);

    setLayout(layout);
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

#else // EnableQt

void msxmInitializeWidget()
{
}

#endif // EnableQt
