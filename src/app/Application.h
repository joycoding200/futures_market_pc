#pragma once
#include <QApplication>
#include <spdlog/spdlog.h>

class SimDataProvider;
class MarketDataBuffer;

class Application : public QApplication {
    Q_OBJECT
public:
    Application(int& argc, char** argv);
    ~Application();
    int run();

private:
    void initLogging();
    void initEventBus();
    void initDataLayer();
    void initMainWindow();

    SimDataProvider* m_dataProvider = nullptr;
    MarketDataBuffer* m_buffer = nullptr;
};
