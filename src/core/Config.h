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

    QString dataSourceType() const;
    void setDataSourceType(const QString& type);
    QString ctpBrokerId() const;
    void setCtpBrokerId(const QString& id);
    QString ctpAddress() const;
    void setCtpAddress(const QString& addr);

private:
    Config();
    std::unique_ptr<QSettings> m_settings;
};
