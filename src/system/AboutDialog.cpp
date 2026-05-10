#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QtCore/qglobal.h>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("关于");
    setFixedSize(320, 200);

    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    auto* title = new QLabel("期货行情PC软件");
    QFont titleFont("Arial", 16, QFont::Bold);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* version = new QLabel("版本 0.2.0");
    version->setAlignment(Qt::AlignCenter);
    layout->addWidget(version);

    auto* qtVer = new QLabel(QString("基于 Qt %1").arg(qVersion()));
    qtVer->setAlignment(Qt::AlignCenter);
    qtVer->setStyleSheet("color: #888;");
    layout->addWidget(qtVer);

    auto* tech = new QLabel("技术栈: Qt 6.x / C++17 / QPainter 自绘");
    tech->setAlignment(Qt::AlignCenter);
    tech->setStyleSheet("color: #888; font-size: 11px;");
    layout->addWidget(tech);

    layout->addSpacing(12);

    auto* closeBtn = new QPushButton("关闭");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn);
}
