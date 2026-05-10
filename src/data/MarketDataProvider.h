#pragma once
#include <QObject>
#include <QString>
#include <QVector>
#include "TickData.h"
#include "KLineData.h"

class MarketDataProvider : public QObject {
    Q_OBJECT
public:
    explicit MarketDataProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~MarketDataProvider() = default;

    virtual void subscribe(const QString& contract) = 0;
    virtual void unsubscribe(const QString& contract) = 0;
    virtual void subscribeAll(const QVector<QString>& contracts) = 0;

    virtual void requestHistory(const QString& contract, KLineType type,
                                int count) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual QString providerName() const = 0;

signals:
    void tickReceived(const TickData& tick);
    void klineReceived(const KLineData& kline);
    void historyLoaded(const QString& contract, const QVector<KLineData>& data);
    void connectionStatus(const QString& status);
    void errorOccurred(const QString& error);
};
