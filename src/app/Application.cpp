#include "Application.h"
#include "core/EventBus.h"
#include "core/Config.h"
#include "data/SimDataProvider.h"
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
    auto* simProvider = new SimDataProvider(this);
    auto* buffer = new MarketDataBuffer(this);
    auto* aggregator = new KLineAggregator(buffer, this);

    QObject::connect(simProvider, &SimDataProvider::tickReceived,
                     buffer, &MarketDataBuffer::onTick);
    QObject::connect(buffer, &MarketDataBuffer::tickUpdated,
                     aggregator, &KLineAggregator::onTick);

    simProvider->start();
    spdlog::debug("数据层初始化完成");
}

int Application::run() {
    initEventBus();
    initDataLayer();
    initMainWindow();
    return exec();
}
