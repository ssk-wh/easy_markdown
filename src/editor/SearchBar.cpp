#include "SearchBar.h"
#include "EditorWidget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>

SearchBar::SearchBar(EditorWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(400);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 4, 8, 4);
    mainLayout->setSpacing(4);

    // 搜索行
    auto* findRow = new QHBoxLayout();
    m_findEdit = new QLineEdit();
    m_findEdit->setPlaceholderText("Search...");
    m_findPrevBtn = new QPushButton("<");
    m_findPrevBtn->setFixedWidth(30);
    m_findNextBtn = new QPushButton(">");
    m_findNextBtn->setFixedWidth(30);
    m_closeBtn = new QPushButton("X");
    m_closeBtn->setFixedWidth(30);
    findRow->addWidget(m_findEdit);
    findRow->addWidget(m_findPrevBtn);
    findRow->addWidget(m_findNextBtn);
    findRow->addWidget(m_closeBtn);
    mainLayout->addLayout(findRow);

    // 替换行（可隐藏）
    m_replaceRow = new QWidget();
    auto* replaceLayout = new QHBoxLayout(m_replaceRow);
    replaceLayout->setContentsMargins(0, 0, 0, 0);
    m_replaceEdit = new QLineEdit();
    m_replaceEdit->setPlaceholderText("Replace...");
    m_replaceBtn = new QPushButton("Replace");
    m_replaceAllBtn = new QPushButton("All");
    replaceLayout->addWidget(m_replaceEdit);
    replaceLayout->addWidget(m_replaceBtn);
    replaceLayout->addWidget(m_replaceAllBtn);
    mainLayout->addWidget(m_replaceRow);
    m_replaceRow->hide();

    // 信号连接
    connect(m_findNextBtn, &QPushButton::clicked, this, [this]() {
        emit findNext(m_findEdit->text());
    });
    connect(m_findPrevBtn, &QPushButton::clicked, this, [this]() {
        emit findPrev(m_findEdit->text());
    });
    connect(m_replaceBtn, &QPushButton::clicked, this, [this]() {
        emit replaceNext(m_findEdit->text(), m_replaceEdit->text());
    });
    connect(m_replaceAllBtn, &QPushButton::clicked, this, [this]() {
        emit replaceAll(m_findEdit->text(), m_replaceEdit->text());
    });
    connect(m_closeBtn, &QPushButton::clicked, this, &SearchBar::hideBar);
    connect(m_findEdit, &QLineEdit::textChanged, this, &SearchBar::searchTextChanged);
    connect(m_findEdit, &QLineEdit::returnPressed, this, [this]() {
        emit findNext(m_findEdit->text());
    });

    // Escape 关闭
    m_findEdit->installEventFilter(this);
    m_replaceEdit->installEventFilter(this);

    // 样式
    setStyleSheet("SearchBar { background: white; border: 1px solid #ccc; border-radius: 4px; }");
    setAutoFillBackground(true);

    hide();
}

bool SearchBar::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Escape) {
            hideBar();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

QString SearchBar::searchText() const
{
    return m_findEdit->text();
}

void SearchBar::showSearch()
{
    m_replaceRow->hide();
    adjustSize();
    QWidget* p = parentWidget();
    move(p->width() - width() - 20, 10);
    show();
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void SearchBar::showReplace()
{
    m_replaceRow->show();
    adjustSize();
    QWidget* p = parentWidget();
    move(p->width() - width() - 20, 10);
    show();
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void SearchBar::hideBar()
{
    hide();
    emit closed();
    parentWidget()->setFocus();
}
