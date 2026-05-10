#include "CTPDataProvider.h"
#include "core/Config.h"
#include <spdlog/spdlog.h>
#include <ThostFtdcMdApi.h>
#include <QDateTime>

static CTPDataProvider* g_ctpInstance = nullptr;

extern "C" {
    static void OnFrontConnected() { if (g_ctpInstance) g_ctpInstance->onFrontConnected(); }
    static void OnFrontDisconnected(int r) { if (g_ctpInstance) g_ctpInstance->onFrontDisconnected(r); }
    static void OnRspUserLogin(void* p, void* i, int n, bool l)
        { if (g_ctpInstance) g_ctpInstance->onRspUserLogin(p, i, n, l); }
    static void OnRspSubMarketData(void* p, void* i, int n, bool l)
        { if (g_ctpInstance) g_ctpInstance->onRspSubMarketData(p, i, n, l); }
    static void OnRtnDepthMarketData(void* p)
        { if (g_ctpInstance) g_ctpInstance->onRtnDepthMarketData(p); }
}

CTPDataProvider::CTPDataProvider(QObject* parent) : MarketDataProvider(parent) {
    auto& cfg = Config::instance();
    m_frontAddr = cfg.ctpAddress();
    m_brokerId = cfg.ctpBrokerId();
    m_userId = "investor";
    m_password = "password";
}

CTPDataProvider::~CTPDataProvider() { stop(); }

void CTPDataProvider::start() {
    g_ctpInstance = this;
    QString flowPath = "./ctp_flow/";
    m_api = CThostFtdcMdApi::CreateFtdcMdApi(flowPath.toStdString().c_str());
    m_api->RegisterSpi(new CThostFtdcMdSpi());
    m_api->RegisterFront(m_frontAddr.toStdString().c_str());
    m_api->Init();
    spdlog::info("CTPDataProvider: 连接 {} ...", m_frontAddr.toStdString());
}

void CTPDataProvider::stop() {
    if (m_api) {
        m_api->RegisterSpi(nullptr);
        m_api->Release();
        m_api = nullptr;
    }
    g_ctpInstance = nullptr;
    m_loggedIn = false;
}

void CTPDataProvider::subscribe(const QString& contract) {
    if (m_loggedIn && m_api) {
        char* codes[1];
        std::string c = contract.toStdString();
        codes[0] = const_cast<char*>(c.c_str());
        m_api->SubscribeMarketData(codes, 1);
    } else {
        if (!m_pendingContracts.contains(contract))
            m_pendingContracts.append(contract);
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

void CTPDataProvider::onFrontConnected() {
    spdlog::info("CTP: 前置连接成功");
    login();
}

void CTPDataProvider::onFrontDisconnected(int reason) {
    spdlog::warn("CTP: 连接断开, reason={}", reason);
    m_loggedIn = false;
}

void CTPDataProvider::onRspUserLogin(void* pRsp, void* pRspInfo, int requestId, bool last) {
    auto* info = static_cast<CThostFtdcRspInfoField*>(pRspInfo);
    if (info && info->ErrorID == 0) {
        m_loggedIn = true;
        spdlog::info("CTP: 登录成功");
        subscribeContracts();
    } else {
        int errId = info ? info->ErrorID : -1;
        spdlog::error("CTP: 登录失败, ErrorID={}", errId);
        emit errorOccurred(QString("CTP登录失败: %1").arg(errId));
    }
}

void CTPDataProvider::onRspSubMarketData(void* pInstrument, void* pRspInfo, int requestId, bool last) {
    auto* inst = static_cast<CThostFtdcSpecificInstrumentField*>(pInstrument);
    auto* info = static_cast<CThostFtdcRspInfoField*>(pRspInfo);
    if (info && info->ErrorID != 0) {
        spdlog::warn("CTP: 订阅失败 {} ErrorID={}", inst ? inst->InstrumentID : "?", info->ErrorID);
    }
}

void CTPDataProvider::onRtnDepthMarketData(void* pData) {
    auto* ctpTick = static_cast<CThostFtdcDepthMarketDataField*>(pData);
    TickData tick = convertTick(ctpTick);
    emit tickReceived(tick);
}

void CTPDataProvider::login() {
    CThostFtdcReqUserLoginField req = {};
    strncpy(req.BrokerID, m_brokerId.toStdString().c_str(), sizeof(req.BrokerID) - 1);
    strncpy(req.UserID, m_userId.toStdString().c_str(), sizeof(req.UserID) - 1);
    strncpy(req.Password, m_password.toStdString().c_str(), sizeof(req.Password) - 1);
    m_api->ReqUserLogin(&req, ++m_requestId);
}

void CTPDataProvider::subscribeContracts() {
    for (const auto& c : m_pendingContracts) {
        char* codes[1];
        std::string cs = c.toStdString();
        codes[0] = const_cast<char*>(cs.c_str());
        m_api->SubscribeMarketData(codes, 1);
    }
}

TickData CTPDataProvider::convertTick(void* pData) const {
    auto* d = static_cast<CThostFtdcDepthMarketDataField*>(pData);
    TickData t;
    t.contract = QString::fromLocal8Bit(d->InstrumentID);
    t.lastPrice = d->LastPrice;
    t.openPrice = d->OpenPrice;
    t.highPrice = d->HighestPrice;
    t.lowPrice = d->LowestPrice;
    t.volume = d->Volume;
    t.openInterest = d->OpenInterest;
    t.preSettle = d->PreSettlementPrice;
    t.bidPrice = {d->BidPrice1, d->BidPrice2, d->BidPrice3, d->BidPrice4, d->BidPrice5};
    t.bidVolume = {d->BidVolume1, d->BidVolume2, d->BidVolume3, d->BidVolume4, d->BidVolume5};
    t.askPrice = {d->AskPrice1, d->AskPrice2, d->AskPrice3, d->AskPrice4, d->AskPrice5};
    t.askVolume = {d->AskVolume1, d->AskVolume2, d->AskVolume3, d->AskVolume4, d->AskVolume5};
    QString timeStr = QString::fromLocal8Bit(d->UpdateTime);
    int ms = d->UpdateMillisec;
    QDateTime dt = QDateTime::fromString(timeStr, "HH:mm:ss");
    dt = dt.addMSecs(ms);
    t.timestamp = dt.toMSecsSinceEpoch();
    return t;
}
