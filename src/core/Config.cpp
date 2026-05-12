#include "Config.h"

Config& Config::instance() {
    static Config c;
    return c;
}

Config::Config()
    : m_settings(std::make_unique<QSettings>("FuturesMarketPC", "config")) {
    if (!m_settings->contains("refreshInterval")) m_settings->setValue("refreshInterval", 250);
    if (!m_settings->contains("maxKLineCount")) m_settings->setValue("maxKLineCount", 2000);
    if (!m_settings->contains("simBasePrice")) m_settings->setValue("simBasePrice", 4000.0);
    if (!m_settings->contains("simVolatility")) m_settings->setValue("simVolatility", 0.001);
    if (!m_settings->contains("dataSourceType")) m_settings->setValue("dataSourceType", "Sim");
    if (!m_settings->contains("ctpBrokerId")) m_settings->setValue("ctpBrokerId", "");
    if (!m_settings->contains("ctpAddress")) m_settings->setValue("ctpAddress", "");
    m_settings->sync();
}

int Config::refreshIntervalMs() const { return m_settings->value("refreshInterval").toInt(); }
void Config::setRefreshIntervalMs(int ms) { m_settings->setValue("refreshInterval", ms); }
int Config::maxKLineCount() const { return m_settings->value("maxKLineCount").toInt(); }
void Config::setMaxKLineCount(int count) { m_settings->setValue("maxKLineCount", count); }
QString Config::defaultContracts() const { return m_settings->value("defaultContracts").toString(); }
void Config::setDefaultContracts(const QString& cs) { m_settings->setValue("defaultContracts", cs); }
double Config::simBasePrice() const { return m_settings->value("simBasePrice").toDouble(); }
double Config::simVolatility() const { return m_settings->value("simVolatility").toDouble(); }
QString Config::dataSourceType() const { return m_settings->value("dataSourceType").toString(); }
void Config::setDataSourceType(const QString& t) { m_settings->setValue("dataSourceType", t); }
QString Config::ctpBrokerId() const { return m_settings->value("ctpBrokerId").toString(); }
void Config::setCtpBrokerId(const QString& id) { m_settings->setValue("ctpBrokerId", id); }
QString Config::ctpAddress() const { return m_settings->value("ctpAddress").toString(); }
void Config::setCtpAddress(const QString& a) { m_settings->setValue("ctpAddress", a); }
