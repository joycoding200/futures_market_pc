#pragma once
#include <QObject>

class EventBus : public QObject {
    Q_OBJECT
public:
    static EventBus& instance();
    enum class EventType {
        TickUpdate,
        KLineUpdate,
        ContractSelected,
        PeriodChanged,
        DataSourceChanged
    };

signals:
    void tickUpdated(const QString& contract, double price, double volume);
    void klineUpdated(const QString& contract, const QVariantMap& kline);
    void contractSelected(const QString& contract);
    void periodChanged(const QString& period);

public:
    void emitTick(const QString& contract, double price, double volume) {
        emit tickUpdated(contract, price, volume);
    }

private:
    EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
};
