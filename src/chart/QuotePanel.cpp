#include "QuotePanel.h"
#include <QFont>

QuotePanel::QuotePanel(QWidget* parent) : QWidget(parent) { setupUI(); }

void QuotePanel::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(4, 4, 4, 4);

    m_priceLabel = new QLabel("----");
    m_priceLabel->setAlignment(Qt::AlignCenter);
    QFont priceFont("Arial", 28, QFont::Bold);
    m_priceLabel->setFont(priceFont);
    m_priceLabel->setStyleSheet("color: #ffffff;");
    layout->addWidget(m_priceLabel);

    m_changeLabel = new QLabel("0.00%");
    m_changeLabel->setAlignment(Qt::AlignCenter);
    m_changeLabel->setStyleSheet("color: #888888; font-size: 14px;");
    layout->addWidget(m_changeLabel);

    for (int i = 0; i < 5; ++i) {
        m_askLabels[i] = new QLabel(QString("卖%1 ----").arg(5 - i));
        m_askLabels[i]->setStyleSheet("color: #ef4444; font-size: 11px;");
        layout->addWidget(m_askLabels[i]);
    }

    auto* sep = new QLabel();
    sep->setFixedHeight(1);
    sep->setStyleSheet("background: #333;");
    layout->addWidget(sep);

    for (int i = 0; i < 5; ++i) {
        m_bidLabels[i] = new QLabel(QString("买%1 ----").arg(i + 1));
        m_bidLabels[i]->setStyleSheet("color: #22c55e; font-size: 11px;");
        layout->addWidget(m_bidLabels[i]);
    }

    m_volumeLabel = new QLabel("量: --");
    m_volumeLabel->setStyleSheet("color: #888; font-size: 11px;");
    layout->addWidget(m_volumeLabel);

    m_oiLabel = new QLabel("仓: --");
    m_oiLabel->setStyleSheet("color: #888; font-size: 11px;");
    layout->addWidget(m_oiLabel);

    setStyleSheet("background: #1a1a24;");
}

void QuotePanel::updateQuote(const TickData& tick) {
    m_priceLabel->setText(QString::number(tick.lastPrice, 'f', 0));
    double changeRate = (tick.lastPrice - tick.preSettle) / tick.preSettle * 100.0;
    QString changeStr = QString("%1%2%").arg(changeRate >= 0 ? "+" : "").arg(changeRate, 0, 'f', 2);
    m_changeLabel->setText(changeStr);
    m_changeLabel->setStyleSheet(
        (changeRate >= 0 ? "color: #ef4444;" : "color: #22c55e;") + QString(" font-size: 14px;"));

    for (int i = 0; i < 5; ++i) {
        m_askLabels[i]->setText(QString("卖%1 %2 %3")
            .arg(5 - i).arg(tick.askPrice[i], 0, 'f', 0).arg(static_cast<int>(tick.askVolume[i])));
        m_bidLabels[i]->setText(QString("买%1 %2 %3")
            .arg(i + 1).arg(tick.bidPrice[i], 0, 'f', 0).arg(static_cast<int>(tick.bidVolume[i])));
    }
    m_volumeLabel->setText(QString("量: %1").arg(static_cast<int>(tick.volume)));
    m_oiLabel->setText(QString("仓: %1").arg(static_cast<int>(tick.openInterest)));
}
