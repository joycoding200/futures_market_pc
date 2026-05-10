#include "Application.h"
#include <QMainWindow>
#include "app/MainWindow.h"
#include "core/EventBus.h"
#include "core/Config.h"
#include "data/SimDataProvider.h"
#include "data/CTPDataProvider.h"
#include "data/MarketDataBuffer.h"
#include "data/KLineAggregator.h"

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv) {
    setApplicationName("期货行情PC软件");
    setApplicationVersion("0.1.0");
    initLogging();
}

Application::~Application() {}

void Application::initLogging() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("期货行情PC软件启动");
}

void Application::initEventBus() {
    EventBus::instance();
    spdlog::debug("EventBus 初始化完成");
}

void Application::initDataLayer() {
    if (Config::instance().dataSourceType() == "CTP") {
        m_dataProvider = new CTPDataProvider(this);
        spdlog::info("使用 CTP 数据源");
    } else {
        m_dataProvider = new SimDataProvider(this);
        spdlog::info("使用模拟数据源");
    }
    m_buffer = new MarketDataBuffer(2000, this);
    auto* aggregator = new KLineAggregator(m_buffer, this);

    QObject::connect(m_dataProvider, &SimDataProvider::tickReceived,
                     m_buffer, &MarketDataBuffer::onTick);
    QObject::connect(m_buffer, &MarketDataBuffer::tickUpdated,
                     aggregator, &KLineAggregator::onTick);
    // 将 Buffer 信号中继到 EventBus，供 UI 层消费
    auto& bus = EventBus::instance();
    QObject::connect(m_buffer, &MarketDataBuffer::tickUpdated,
                     &bus, [&bus](const TickData& tick) {
        bus.emitTick(tick.contract, tick.lastPrice, tick.volume);
    });
    m_dataProvider->start();
    spdlog::debug("数据层初始化完成");
}

void Application::initMainWindow() {
    auto* window = new MainWindow(m_buffer, m_dataProvider);
    window->show();
}

int Application::run() {
    initEventBus();
    initDataLayer();
    initMainWindow();
    return exec();
}
