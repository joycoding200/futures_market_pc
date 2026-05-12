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

    // 搜索和筛选栏
    auto* filterRow = new QHBoxLayout();
    m_searchBox = new QLineEdit();
    m_searchBox->setPlaceholderText("搜索合约...");
    m_searchBox->setClearButtonEnabled(true);
    m_searchBox->setStyleSheet(
        "QLineEdit { background: #1e1e2a; color: #ccc; border: 1px solid #333; padding: 4px; }");

    m_exchangeFilter = new QComboBox();
    m_exchangeFilter->addItems({"全部", "CFFEX", "SHFE", "DCE", "CZCE", "INE"});
    m_exchangeFilter->setStyleSheet(
        "QComboBox { background: #1e1e2a; color: #ccc; border: 1px solid #333; padding: 4px; }");

    filterRow->addWidget(m_searchBox);
    filterRow->addWidget(m_exchangeFilter);
    layout->insertLayout(0, filterRow);  // Insert at top, before m_tree

    // 启用表头排序
    m_tree->setSortingEnabled(true);
    m_tree->header()->setSortIndicatorShown(true);

    // 连接筛选信号
    connect(m_searchBox, &QLineEdit::textChanged, this, &ContractList::applyFilter);
    connect(m_exchangeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { applyFilter(); });

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
    item->setForeground(1, changeRate >= 0 ? QColor(239, 68, 68) : QColor(34, 197, 94));
    item->setForeground(2, changeRate >= 0 ? QColor(239, 68, 68) : QColor(34, 197, 94));
}

void ContractList::applyFilter() {
    QString keyword = m_searchBox->text().trimmed();
    QString exchange = m_exchangeFilter->currentText();
    if (exchange == "全部") exchange.clear();

    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        auto* item = m_tree->topLevelItem(i);
        bool match = item->text(0).contains(keyword, Qt::CaseInsensitive);
        if (!match) {
            item->setHidden(true);
        } else {
            item->setHidden(false);
        }
    }
}
