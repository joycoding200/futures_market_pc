#pragma once
#include <QString>
#include <QSettings>
#include <memory>

class Config {
public:
    static Config& instance();

    int refreshIntervalMs() const;
    void setRefreshIntervalMs(int ms);

    int maxKLineCount() const;
    void setMaxKLineCount(int count);

    QString defaultContracts() const;
    void setDefaultContracts(const QString& contracts);

    double simBasePrice() const;
    double simVolatility() const;

private:
    Config();
    std::unique_ptr<QSettings> m_settings;
};
