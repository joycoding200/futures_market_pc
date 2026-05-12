#pragma once
#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>
#include "data/TickData.h"

// 单合约统计快照
struct ContractStats {
    QString  contract;
    double   lastPrice    = 0.0;
    double   preSettle    = 0.0;
    double   changeRate   = 0.0;   // 涨跌幅 %
    double   volume       = 0.0;
    double   openInterest = 0.0;
    double   highPrice    = 0.0;
    double   lowPrice     = 999999.0;
    double   amplitude    = 0.0;   // 振幅 %
    QString  exchange;             // 从合约代码前缀提取

    double change() const { return lastPrice - preSettle; }
};

// 成交量分布统计
struct VolumeDistribution {
    int largeCount  = 0;   // >= 100 手
    int mediumCount = 0;   // 30-99 手
    int smallCount  = 0;   // <30 手
    int totalCount  = 0;

    void addTrade(int volume) {
        if (volume >= 100) largeCount++;
        else if (volume >= 30) mediumCount++;
        else smallCount++;
        totalCount++;
    }
};

enum class SortField { Contract, Price, ChangeRate, Volume, Amplitude };

// 统计计算引擎
// 线程安全: onTick() 和查询方法必须在同一线程调用 (通过 Qt::QueuedConnection 保证)
class StatsCalculator : public QObject {
    Q_OBJECT
public:
    explicit StatsCalculator(QObject* parent = nullptr);

    // 更新 Tick → 内部增量计算
    void onTick(const TickData& tick);

    // 查询接口
    QVector<ContractStats> topGainers(int count = 10) const;   // 涨幅Top N
    QVector<ContractStats> topLosers(int count = 10) const;    // 跌幅Top N
    QVector<ContractStats> topVolume(int count = 10) const;    // 成交量Top N
    QVector<ContractStats> allStats(SortField sortBy = SortField::ChangeRate,
                                     Qt::SortOrder order = Qt::DescendingOrder) const;
    ContractStats statsFor(const QString& contract) const;
    VolumeDistribution volumeDist() const { return m_volDist; }

    // 筛选
    QVector<ContractStats> filter(const QString& keyword, const QString& exchange = "",
                                   int direction = 0) const; // -1跌 0全部 +1涨

signals:
    void statsUpdated();  // 统计刷新后发射

private:
    QString extractExchange(const QString& contract) const;
    QMap<QString, ContractStats> m_stats;
    VolumeDistribution m_volDist;
};
