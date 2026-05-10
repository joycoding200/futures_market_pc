#pragma once
#include <QString>
#include "MarketDataProvider.h"

class CThostFtdcMdApi;
class CThostFtdcMdSpi;
struct CThostFtdcDepthMarketDataField;
struct CThostFtdcRspUserLoginField;
struct CThostFtdcRspInfoField;
struct CThostFtdcSpecificInstrumentField;

class CTPDataProvider : public MarketDataProvider {
    Q_OBJECT
public:
    explicit CTPDataProvider(QObject* parent = nullptr);
    ~CTPDataProvider() override;

    void subscribe(const QString& contract) override;
    void unsubscribe(const QString& contract) override;
    void subscribeAll(const QVector<QString>& contracts) override;
    void requestHistory(const QString& contract, KLineType type, int count) override;
    void start() override;
    void stop() override;
    QString providerName() const override { return "CTP"; }

    // 供 SpiBridge 调用的回调处理
    void handleFrontConnected();
    void handleFrontDisconnected(int reason);
    void handleRspUserLogin(CThostFtdcRspUserLoginField* pRsp, CThostFtdcRspInfoField* pInfo,
                            int requestId, bool isLast);
    void handleRspSubMarketData(CThostFtdcSpecificInstrumentField* pInst, CThostFtdcRspInfoField* pInfo,
                                int requestId, bool isLast);
    void handleRtnDepthMarketData(CThostFtdcDepthMarketDataField* pData);

private:
    void login();
    void subscribeContracts();
    TickData convertTick(CThostFtdcDepthMarketDataField* d) const;

    CThostFtdcMdApi* m_api = nullptr;
    CThostFtdcMdSpi*  m_spi = nullptr;
    QStringList m_pendingContracts;
    bool m_loggedIn = false;

    QString m_frontAddr;
    QString m_brokerId;
    QString m_userId;
    QString m_password;
    int m_requestId = 0;
};
