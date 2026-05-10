#include "SettingsDialog.h"
#include "core/Config.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("系统设置");
    setMinimumWidth(450);
    setupUI();
    loadConfig();
}

void SettingsDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    auto* tabs = new QTabWidget();

    auto* marketTab = new QWidget();
    auto* marketForm = new QFormLayout(marketTab);

    m_refreshInterval = new QSpinBox();
    m_refreshInterval->setRange(50, 2000);
    m_refreshInterval->setSuffix(" ms");
    marketForm->addRow("刷新间隔:", m_refreshInterval);

    m_maxKLineCount = new QSpinBox();
    m_maxKLineCount->setRange(100, 10000);
    marketForm->addRow("K线最大数量:", m_maxKLineCount);

    m_simBasePrice = new QDoubleSpinBox();
    m_simBasePrice->setRange(1, 99999);
    m_simBasePrice->setDecimals(0);
    marketForm->addRow("模拟基准价:", m_simBasePrice);

    m_simVolatility = new QDoubleSpinBox();
    m_simVolatility->setRange(0.1, 100);
    m_simVolatility->setDecimals(1);
    marketForm->addRow("波动幅度:", m_simVolatility);

    tabs->addTab(marketTab, "行情");

    auto* sourceTab = new QWidget();
    auto* sourceForm = new QFormLayout(sourceTab);

    m_dataSourceType = new QComboBox();
    m_dataSourceType->addItem("模拟数据 (Sim)", "Sim");
    m_dataSourceType->addItem("CTP", "CTP");
    m_dataSourceType->addItem("HTTP", "HTTP");
    sourceForm->addRow("数据源类型:", m_dataSourceType);

    m_ctpFields = new QWidget();
    auto* ctpForm = new QFormLayout(m_ctpFields);
    ctpForm->setContentsMargins(0, 0, 0, 0);
    m_ctpBrokerId = new QLineEdit();
    ctpForm->addRow("Broker ID:", m_ctpBrokerId);
    m_ctpAddress = new QLineEdit();
    ctpForm->addRow("行情地址:", m_ctpAddress);
    sourceForm->addRow(m_ctpFields);

    m_statusLabel = new QLabel("当前: 模拟数据源 (SimDataProvider)");
    sourceForm->addRow("状态:", m_statusLabel);

    tabs->addTab(sourceTab, "数据源");

    mainLayout->addWidget(tabs);

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(btnBox);

    connect(m_dataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onDataSourceChanged);
}

void SettingsDialog::loadConfig() {
    auto& cfg = Config::instance();
    m_refreshInterval->setValue(cfg.refreshIntervalMs());
    m_maxKLineCount->setValue(cfg.maxKLineCount());
    m_simBasePrice->setValue(cfg.simBasePrice());
    m_simVolatility->setValue(cfg.simVolatility());
    int idx = m_dataSourceType->findData(cfg.dataSourceType());
    m_dataSourceType->setCurrentIndex(idx >= 0 ? idx : 0);
    m_ctpBrokerId->setText(cfg.ctpBrokerId());
    m_ctpAddress->setText(cfg.ctpAddress());
    onDataSourceChanged(m_dataSourceType->currentIndex());
}

void SettingsDialog::saveConfig() {
    auto& cfg = Config::instance();
    cfg.setRefreshIntervalMs(m_refreshInterval->value());
    cfg.setMaxKLineCount(m_maxKLineCount->value());
    cfg.setDataSourceType(m_dataSourceType->currentData().toString());
}

void SettingsDialog::onAccepted() {
    saveConfig();
    accept();
}

void SettingsDialog::onDataSourceChanged(int index) {
    QString type = m_dataSourceType->itemData(index).toString();
    m_ctpFields->setVisible(type == "CTP");
}
