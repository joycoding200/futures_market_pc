#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QStackedWidget>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onDataSourceChanged(int index);
    void onAccepted();

private:
    void setupUI();
    void loadConfig();
    void saveConfig();

    QSpinBox* m_refreshInterval;
    QSpinBox* m_maxKLineCount;
    QDoubleSpinBox* m_simBasePrice;
    QDoubleSpinBox* m_simVolatility;
    QComboBox* m_dataSourceType;
    QLineEdit* m_ctpBrokerId;
    QLineEdit* m_ctpAddress;
    QWidget* m_ctpFields;
    QLabel* m_statusLabel;
};
