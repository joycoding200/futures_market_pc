#include "CTPDataProvider.h"
#include "core/Config.h"
#include <spdlog/spdlog.h>
#include <ThostFtdcMdApi.h>
#include <QDateTime>

// SpiBridge — 继承 CThostFtdcMdSpi，将 C++ 虚函数回调转发到 CTPDataProvider
class SpiBridge : public CThostFtdcMdSpi {
public:
    explicit SpiBridge(CTPDataProvider* owner) : m_owner(owner) {}

    void OnFrontConnected() override {
        m_owner->handleFrontConnected();
    }
    void OnFrontDisconnected(int reason) override {
        m_owner->handleFrontDisconnected(reason);
    }
    void OnRspUserLogin(CThostFtdcRspUserLoginField* pRsp, CThostFtdcRspInfoField* pInfo,
                        int nRequestID, bool bIsLast) override {
        m_owner->handleRspUserLogin(pRsp, pInfo, nRequestID, bIsLast);
    }
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                            CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override {
        m_owner->handleRspSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast);
    }
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) override {
        m_owner->handleRtnDepthMarketData(pDepthMarketData);
    }

private:
    CTPDataProvider* m_owner;
};

// ============================================================================
// CTPDataProvider 实现
// ============================================================================

CTPDataProvider::CTPDataProvider(QObject* parent) : MarketDataProvider(parent) {
    auto& cfg = Config::instance();
    m_frontAddr = cfg.ctpAddress();
    m_brokerId = cfg.ctpBrokerId();
    // 用户凭证 —— 当前从 Config 读取，需在设置中配置
    m_userId = "investor";
    m_password = "password";
}

CTPDataProvider::~CTPDataProvider() { stop(); }

void CTPDataProvider::start() {
    QString flowPath = "./ctp_flow/";
    m_api = CThostFtdcMdApi::CreateFtdcMdApi(flowPath.toLocal8Bit().data());
    m_spi = new SpiBridge(this);
    m_api->RegisterSpi(m_spi);
    m_api->RegisterFront(m_frontAddr.toLocal8Bit().data());
    m_api->Init();
    spdlog::info("CTPDataProvider: 连接 {} ...", m_frontAddr.toStdString());
}

void CTPDataProvider::stop() {
    if (m_api) {
        m_api->RegisterSpi(nullptr);
        m_api->Release();
        m_api = nullptr;
    }
    delete m_spi;
    m_spi = nullptr;
    m_loggedIn = false;
}

void CTPDataProvider::subscribe(const QString& contract) {
    if (m_loggedIn && m_api) {
        char* codes[1];
        std::string c = contract.toStdString();
        codes[0] = const_cast<char*>(c.c_str());
        m_api->SubscribeMarketData(codes, 1);
        spdlog::info("CTP: 订阅 {}", c);
    } else {
        if (!m_pendingContracts.contains(contract))
            m_pendingContracts.append(contract);
        spdlog::debug("CTP: 缓存订阅 {}", contract.toStdString());
    }
}

void CTPDataProvider::unsubscribe(const QString& contract) {
    if (m_api && m_loggedIn) {
        char* codes[1];
        std::string c = contract.toStdString();
        codes[0] = const_cast<char*>(c.c_str());
        m_api->UnSubscribeMarketData(codes, 1);
    }
    m_pendingContracts.removeAll(contract);
}

void CTPDataProvider::subscribeAll(const QVector<QString>& contracts) {
    for (const auto& c : contracts) subscribe(c);
}

void CTPDataProvider::requestHistory(const QString& contract, KLineType type, int count) {
    Q_UNUSED(contract); Q_UNUSED(type); Q_UNUSED(count);
}

// ============================================================================
// 回调处理
// ============================================================================

void CTPDataProvider::handleFrontConnected() {
    spdlog::info("CTP: 前置连接成功");
    login();
}

void CTPDataProvider::handleFrontDisconnected(int reason) {
    spdlog::warn("CTP: 连接断开, reason={}", reason);
    m_loggedIn = false;
    emit connectionStatus("已断开");
}

void CTPDataProvider::handleRspUserLogin(CThostFtdcRspUserLoginField* pRsp,
                                          CThostFtdcRspInfoField* pInfo,
                                          int requestId, bool isLast) {
    if (pInfo && pInfo->ErrorID == 0) {
        m_loggedIn = true;
        spdlog::info("CTP: 登录成功 (交易日: {})", pRsp ? QString::fromLocal8Bit(pRsp->TradingDay).toStdString() : "?");
        emit connectionStatus("已登录");
        subscribeContracts();
    } else {
        int errId = pInfo ? pInfo->ErrorID : -1;
        spdlog::error("CTP: 登录失败, ErrorID={}", errId);
        emit errorOccurred(QString("CTP登录失败: %1").arg(errId));
    }
}

void CTPDataProvider::handleRspSubMarketData(CThostFtdcSpecificInstrumentField* pInst,
                                              CThostFtdcRspInfoField* pInfo,
                                              int requestId, bool isLast) {
    if (pInfo && pInfo->ErrorID != 0) {
        spdlog::warn("CTP: 订阅失败 {} ErrorID={}",
                     pInst ? QString::fromLocal8Bit(pInst->InstrumentID).toStdString() : "?",
                     pInfo->ErrorID);
    } else if (pInst) {
        spdlog::info("CTP: 订阅成功 {}", QString::fromLocal8Bit(pInst->InstrumentID).toStdString());
    }
}

void CTPDataProvider::handleRtnDepthMarketData(CThostFtdcDepthMarketDataField* pData) {
    TickData tick = convertTick(pData);
    emit tickReceived(tick);
}

// ============================================================================
// 内部方法
// ============================================================================

void CTPDataProvider::login() {
    CThostFtdcReqUserLoginField req = {};
    strncpy(req.BrokerID, m_brokerId.toLocal8Bit().data(), sizeof(req.BrokerID) - 1);
    strncpy(req.UserID, m_userId.toLocal8Bit().data(), sizeof(req.UserID) - 1);
    strncpy(req.Password, m_password.toLocal8Bit().data(), sizeof(req.Password) - 1);
    m_api->ReqUserLogin(&req, ++m_requestId);
    spdlog::info("CTP: 发送登录请求 BrokerID={}", m_brokerId.toStdString());
}

void CTPDataProvider::subscribeContracts() {
    if (m_pendingContracts.isEmpty()) {
        spdlog::warn("CTP: 无待订阅合约");
        return;
    }
    for (const auto& c : m_pendingContracts) {
        char* codes[1];
        std::string cs = c.toStdString();
        codes[0] = const_cast<char*>(cs.c_str());
        m_api->SubscribeMarketData(codes, 1);
        spdlog::info("CTP: 订阅 {}", cs);
    }
}

TickData CTPDataProvider::convertTick(CThostFtdcDepthMarketDataField* d) const {
    TickData t;
    // reserve1 = InstrumentID (新版CTP兼容字段)
    t.contract = QString::fromLocal8Bit(d->reserve1);
    t.lastPrice = d->LastPrice;
    t.openPrice = d->OpenPrice;
    t.highPrice = d->HighestPrice;
    t.lowPrice = d->LowestPrice;
    t.volume = d->Volume;
    t.openInterest = d->OpenInterest;
    t.preSettle = d->PreSettlementPrice;
    t.bidPrice  = {d->BidPrice1, d->BidPrice2, d->BidPrice3, d->BidPrice4, d->BidPrice5};
    t.bidVolume = {d->BidVolume1, d->BidVolume2, d->BidVolume3, d->BidVolume4, d->BidVolume5};
    t.askPrice  = {d->AskPrice1, d->AskPrice2, d->AskPrice3, d->AskPrice4, d->AskPrice5};
    t.askVolume = {d->AskVolume1, d->AskVolume2, d->AskVolume3, d->AskVolume4, d->AskVolume5};

    QString timeStr = QString::fromLocal8Bit(d->UpdateTime);
    int ms = d->UpdateMillisec;
    if (timeStr.isEmpty()) {
        t.timestamp = QDateTime::currentMSecsSinceEpoch();
    } else {
        QDateTime dt = QDateTime::currentDateTime();
        QTime tm = QTime::fromString(timeStr, "HH:mm:ss");
        dt.setTime(tm);
        dt = dt.addMSecs(ms);
        t.timestamp = dt.toMSecsSinceEpoch();
    }
    return t;
}
