#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QComboBox>
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
    void applyFilter();
    static QString extractExchange(const QString& contract);

    QTreeWidget* m_tree;
    QLineEdit* m_searchBox;
    QComboBox* m_exchangeFilter;
    QMap<QString, QTreeWidgetItem*> m_items;
};
