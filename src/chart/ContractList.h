#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QMap>

class ContractList : public QWidget {
    Q_OBJECT
public:
    explicit ContractList(QWidget* parent = nullptr);
    void addContract(const QString& contract, const QString& name = "");
    void updatePrice(const QString& contract, double price, double changeRate);

signals:
    void contractClicked(const QString& contract);

private:
    QTreeWidget* m_tree;
    QMap<QString, QTreeWidgetItem*> m_items;
};
