#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "data/TickData.h"

class QuotePanel : public QWidget {
    Q_OBJECT
public:
    explicit QuotePanel(QWidget* parent = nullptr);
    void updateQuote(const TickData& tick);

private:
    void setupUI();
    QLabel* m_priceLabel;
    QLabel* m_changeLabel;
    QLabel* m_askLabels[5];
    QLabel* m_bidLabels[5];
    QLabel* m_volumeLabel;
    QLabel* m_oiLabel;
};
