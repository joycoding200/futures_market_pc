#include "ContractList.h"
#include <QVBoxLayout>
#include <QHeaderView>

ContractList::ContractList(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeWidget();
    m_tree->setHeaderLabels({"合约", "最新价", "涨跌幅"});
    m_tree->setColumnWidth(0, 80);
    m_tree->setColumnWidth(1, 70);
    m_tree->setColumnWidth(2, 65);
    m_tree->setRootIsDecorated(false);
    m_tree->setStyleSheet(
        "QTreeWidget { background: #161620; color: #ccc; border: none; }"
        "QTreeWidget::item { height: 24px; }"
        "QHeaderView::section { background: #1e1e2a; color: #888; border: none; padding: 4px; }"
    );

    connect(m_tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item) {
        emit contractClicked(item->text(0));
    });

    layout->addWidget(m_tree);
}

void ContractList::addContract(const QString& contract, const QString& name) {
    auto* item = new QTreeWidgetItem();
    item->setText(0, contract);
    item->setToolTip(0, name.isEmpty() ? contract : name);
    item->setText(1, "----");
    item->setText(2, "--");
    m_tree->addTopLevelItem(item);
    m_items[contract] = item;
}

void ContractList::updatePrice(const QString& contract, double price, double changeRate) {
    if (!m_items.contains(contract)) return;
    auto* item = m_items[contract];
    item->setText(1, QString::number(price, 'f', 0));
    item->setText(2, QString("%1%").arg(changeRate, 0, 'f', 2, QChar('+')));
    item->setTextColor(1, changeRate >= 0 ? QColor(239, 68, 68) : QColor(34, 197, 94));
    item->setTextColor(2, changeRate >= 0 ? QColor(239, 68, 68) : QColor(34, 197, 94));
}
