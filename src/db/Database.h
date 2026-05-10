#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include "data/KLineData.h"

class Database : public QObject {
    Q_OBJECT
public:
    explicit Database(const QString& dbPath, QObject* parent = nullptr);
    ~Database();

    bool init();

    void saveKLine(const KLineData& kline);
    QVector<KLineData> loadKLines(const QString& contract, KLineType type,
                                   int64_t from, int64_t to);

private:
    void createTables();
    QSqlDatabase m_db;
    QString m_dbPath;
};
