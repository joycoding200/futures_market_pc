#pragma once
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>

class ContractManager : public QDialog {
    Q_OBJECT
public:
    explicit ContractManager(QWidget* parent = nullptr);
    QStringList contracts() const;

private slots:
    void onAddContract();

private:
    QListWidget* m_list;
    QLineEdit* m_codeInput;
    QLineEdit* m_nameInput;
    void loadContracts();
};
