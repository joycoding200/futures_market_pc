#pragma once
#include <QString>
#include "MarketDataProvider.h"

class CThostFtdcMdApi;

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

    void onFrontConnected();
    void onFrontDisconnected(int reason);
    void onRspUserLogin(void* pRsp, void* pRspInfo, int requestId, bool last);
    void onRspSubMarketData(void* pInstrument, void* pRspInfo, int requestId, bool last);
    void onRtnDepthMarketData(void* pData);

private:
    void login();
    void subscribeContracts();
    TickData convertTick(void* pData) const;

    CThostFtdcMdApi* m_api = nullptr;
    QStringList m_pendingContracts;
    bool m_loggedIn = false;

    QString m_frontAddr;
    QString m_brokerId;
    QString m_userId;
    QString m_password;
    int m_requestId = 0;
};
