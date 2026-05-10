#include "SimDataProvider.h"
#include "core/Config.h"
#include <spdlog/spdlog.h>
#include <cmath>

SimDataProvider::SimDataProvider(QObject* parent)
    : MarketDataProvider(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &SimDataProvider::onTimerTick);
}

SimDataProvider::~SimDataProvider() { stop(); }

void SimDataProvider::subscribe(const QString& contract) {
    if (!m_prices.contains(contract)) {
        m_prices[contract] = Config::instance().simBasePrice();
        m_opens[contract] = Config::instance().simBasePrice();
        spdlog::info("SimDataProvider: 订阅合约 {}", contract.toStdString());
    }
}

void SimDataProvider::unsubscribe(const QString& contract) {
    m_prices.remove(contract);
    m_opens.remove(contract);
}

void SimDataProvider::subscribeAll(const QVector<QString>& contracts) {
    for (const auto& c : contracts) subscribe(c);
}

void SimDataProvider::requestHistory(const QString& contract, KLineType type, int count) {
    Q_UNUSED(contract); Q_UNUSED(type); Q_UNUSED(count);
}

void SimDataProvider::start() {
    m_timer->start(DEFAULT_INTERVAL_MS);
    spdlog::info("SimDataProvider 已启动，刷新间隔 {}ms", DEFAULT_INTERVAL_MS);
}

void SimDataProvider::stop() {
    m_timer->stop();
    spdlog::info("SimDataProvider 已停止");
}

void SimDataProvider::onTimerTick() {
    for (auto it = m_prices.begin(); it != m_prices.end(); ++it) {
        TickData tick = generateTick(it.key());
        it.value() = tick.lastPrice;
        emit tickReceived(tick);
    }
}

TickData SimDataProvider::generateTick(const QString& contract) {
    TickData tick;
    tick.contract = contract;
    tick.timestamp = QDateTime::currentMSecsSinceEpoch();

    double currentPrice = m_prices.value(contract);
    double basePrice = Config::instance().simBasePrice();
    tick.lastPrice = randomWalk(currentPrice, DEFAULT_VOLATILITY,
                                basePrice * 0.85, basePrice * 1.15);
    tick.openPrice = m_opens.value(contract);
    tick.highPrice = std::max(tick.highPrice, tick.lastPrice);
    tick.lowPrice = tick.lowPrice > 0 ? std::min(tick.lowPrice, tick.lastPrice) : tick.lastPrice;
    tick.volume = m_rng.bounded(1, 50);
    tick.openInterest = 100000 + m_rng.bounded(-500, 500);
    tick.preSettle = basePrice;

    double spread = basePrice * 0.0001;
    for (int i = 0; i < 5; ++i) {
        tick.askPrice[i] = tick.lastPrice + spread * (i + 1);
        tick.askVolume[i] = m_rng.bounded(100, 5000);
        tick.bidPrice[i] = tick.lastPrice - spread * (i + 1);
        tick.bidVolume[i] = m_rng.bounded(100, 5000);
    }
    return tick;
}

double SimDataProvider::randomWalk(double currentPrice, double volatility, double min, double max) {
    double change = (m_rng.generateDouble() - 0.5) * 2.0 * volatility;
    double newPrice = currentPrice + change;
    return std::clamp(newPrice, min, max);
}
