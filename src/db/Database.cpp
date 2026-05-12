#include "Database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <spdlog/spdlog.h>

Database::Database(const QString& dbPath, QObject* parent)
    : QObject(parent), m_dbPath(dbPath) {}

Database::~Database() { if (m_db.isOpen()) m_db.close(); }

bool Database::init() {
    m_db = QSqlDatabase::addDatabase("QSQLITE", "main_connection");
    m_db.setDatabaseName(m_dbPath);
    if (!m_db.open()) {
        spdlog::error("数据库打开失败: {}", m_db.lastError().text().toStdString());
        return false;
    }
    createTables();
    spdlog::info("数据库初始化完成: {}", m_dbPath.toStdString());
    return true;
}

void Database::createTables() {
    QSqlQuery query(m_db);
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS klines (
            contract    TEXT NOT NULL,
            type        TEXT NOT NULL,
            timestamp   INTEGER NOT NULL,
            open        REAL,
            high        REAL,
            low         REAL,
            close       REAL,
            volume      REAL,
            open_interest REAL,
            PRIMARY KEY (contract, type, timestamp)
        )
    )");
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS watchlist (
            contract TEXT PRIMARY KEY,
            name     TEXT,
            added_at INTEGER DEFAULT (strftime('%s','now'))
        )
    )");
}

void Database::saveKLine(const KLineData& kline) {
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO klines
        (contract, type, timestamp, open, high, low, close, volume, open_interest)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(kline.contract);
    query.addBindValue(klineTypeToString(kline.type));
    query.addBindValue(kline.timestamp);
    query.addBindValue(kline.open);
    query.addBindValue(kline.high);
    query.addBindValue(kline.low);
    query.addBindValue(kline.close);
    query.addBindValue(kline.volume);
    query.addBindValue(kline.openInterest);
    if (!query.exec()) {
        spdlog::warn("K线保存失败: {}", query.lastError().text().toStdString());
    }
}

QVector<KLineData> Database::loadKLines(const QString& contract, KLineType type,
                                          int64_t from, int64_t to) {
    QVector<KLineData> result;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT * FROM klines WHERE contract = ? AND type = ?
        AND timestamp >= ? AND timestamp <= ? ORDER BY timestamp ASC
    )");
    query.addBindValue(contract);
    query.addBindValue(klineTypeToString(type));
    query.addBindValue(from);
    query.addBindValue(to);

    if (query.exec()) {
        while (query.next()) {
            KLineData k;
            k.contract = query.value(0).toString();
            k.timestamp = query.value(2).toLongLong();
            k.open = query.value(3).toDouble();
            k.high = query.value(4).toDouble();
            k.low = query.value(5).toDouble();
            k.close = query.value(6).toDouble();
            k.volume = query.value(7).toDouble();
            k.openInterest = query.value(8).toDouble();
            k.type = type;
            k.isComplete = true;
            result.append(k);
        }
    }
    return result;
}
