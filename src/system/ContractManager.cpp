#include "ContractManager.h"
#include "core/Config.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>

ContractManager::ContractManager(QWidget* parent) : QDialog(parent) {
    setWindowTitle("合约管理");
    setMinimumSize(350, 400);

    auto* layout = new QVBoxLayout(this);

    m_list = new QListWidget();
    layout->addWidget(new QLabel("已订阅合约 (双击删除):"));
    layout->addWidget(m_list);

    auto* addRow = new QHBoxLayout();
    m_codeInput = new QLineEdit();
    m_codeInput->setPlaceholderText("合约代码 (如 rb2510)");
    m_nameInput = new QLineEdit();
    m_nameInput->setPlaceholderText("合约名称 (如 螺纹钢主连)");
    auto* addBtn = new QPushButton("添加");
    addRow->addWidget(m_codeInput);
    addRow->addWidget(m_nameInput);
    addRow->addWidget(addBtn);
    layout->addLayout(addRow);

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(btnBox);

    connect(addBtn, &QPushButton::clicked, this, &ContractManager::onAddContract);
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        delete item;
    });

    loadContracts();
}

void ContractManager::loadContracts() {
    QStringList codes = Config::instance().defaultContracts().split(",", Qt::SkipEmptyParts);
    for (const auto& c : codes) {
        m_list->addItem(c.trimmed());
    }
}

void ContractManager::onAddContract() {
    QString code = m_codeInput->text().trimmed();
    if (code.isEmpty()) return;
    QString name = m_nameInput->text().trimmed();
    QString display = name.isEmpty() ? code : code + "  " + name;
    m_list->addItem(display);
    m_codeInput->clear();
    m_nameInput->clear();
    m_codeInput->setFocus();
}

QStringList ContractManager::contracts() const {
    QStringList result;
    for (int i = 0; i < m_list->count(); ++i) {
        result.append(m_list->item(i)->text().split(" ").first());
    }
    return result;
}
