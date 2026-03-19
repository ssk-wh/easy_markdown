#include "SearchBar.h"
#include "EditorWidget.h"

#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QMouseEvent>

static constexpr int kBtnW = 28;
static constexpr int kBtnGap = 2;
static constexpr int kReplBtnW = 42;

SearchBar::SearchBar(EditorWidget* parent)
    : QWidget(parent)
    , m_theme(Theme::light())
{
    setFixedWidth(420);
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(6);

    int findBtnsWidth = kBtnW * 3 + kBtnGap * 2;

    // 搜索行
    auto* findRow = new QHBoxLayout();
    findRow->setSpacing(6);
    m_findEdit = new QLineEdit();
    m_findEdit->setPlaceholderText("Search...");
    m_matchInfoLabel = new QLabel();
    m_matchInfoLabel->setFixedWidth(60);
    m_matchInfoLabel->setAlignment(Qt::AlignCenter);

    findRow->addWidget(m_findEdit);
    findRow->addWidget(m_matchInfoLabel);
    findRow->addSpacing(findBtnsWidth);
    mainLayout->addLayout(findRow);

    // 替换行
    m_replaceRow = new QWidget();
    auto* replaceLayout = new QHBoxLayout(m_replaceRow);
    replaceLayout->setContentsMargins(0, 0, 0, 0);
    replaceLayout->setSpacing(6);
    m_replaceEdit = new QLineEdit();
    m_replaceEdit->setPlaceholderText("Replace...");
    replaceLayout->addWidget(m_replaceEdit);
    int replBtnsWidth = kReplBtnW * 2 + kBtnGap + 60; // match info width + replace buttons
    replaceLayout->addSpacing(replBtnsWidth);
    mainLayout->addWidget(m_replaceRow);
    m_replaceRow->hide();

    // 信号连接
    connect(m_findEdit, &QLineEdit::textChanged, this, &SearchBar::searchTextChanged);
    connect(m_findEdit, &QLineEdit::returnPressed, this, [this]() {
        emit findNext(m_findEdit->text());
    });

    // Escape 关闭
    m_findEdit->installEventFilter(this);
    m_replaceEdit->installEventFilter(this);

    applyThemeStyles();
    hide();
}

void SearchBar::applyThemeStyles()
{
    bool dark = m_theme.isDark;

    QColor inputBg = dark ? QColor(60, 60, 60) : QColor(255, 255, 255);
    QColor inputFg = dark ? QColor(220, 220, 220) : QColor(51, 51, 51);
    QColor inputBorder = dark ? QColor(80, 80, 80) : QColor(200, 200, 200);
    QColor labelColor = dark ? QColor(160, 160, 160) : QColor(120, 120, 120);

    QString editStyle = QString(
        "QLineEdit {"
        "  background: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 3px;"
        "  padding: 4px 6px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: %4;"
        "}"
    ).arg(inputBg.name(), inputFg.name(), inputBorder.name(),
          dark ? "#0078D7" : "#0366D6");

    m_findEdit->setStyleSheet(editStyle);
    m_replaceEdit->setStyleSheet(editStyle);

    m_matchInfoLabel->setStyleSheet(QString(
        "QLabel { color: %1; font-size: 12px; background: transparent; border: none; }"
    ).arg(labelColor.name()));
}

void SearchBar::setTheme(const Theme& theme)
{
    m_theme = theme;
    applyThemeStyles();
    update();
}

void SearchBar::updateMatchInfo(int currentIndex, int totalMatches)
{
    if (totalMatches == 0) {
        if (m_findEdit->text().isEmpty())
            m_matchInfoLabel->setText("");
        else
            m_matchInfoLabel->setText("0 / 0");
    } else {
        if (currentIndex < 0)
            m_matchInfoLabel->setText(QString("  %1").arg(totalMatches));
        else
            m_matchInfoLabel->setText(QString("%1 / %2").arg(currentIndex + 1).arg(totalMatches));
    }
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
    m_replaceVisible = false;
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
    m_replaceVisible = true;
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

void SearchBar::updateButtonRects()
{
    int rowH = kBtnW;
    int rightMargin = 10;
    int topMargin = 8;

    // 搜索行按钮：紧贴右侧
    int x = width() - rightMargin - kBtnW;
    int y = topMargin + 1;

    m_btnClose.rect = QRect(x, y, kBtnW, rowH);
    x -= kBtnW + kBtnGap;
    m_btnNext.rect = QRect(x, y, kBtnW, rowH);
    x -= kBtnW + kBtnGap;
    m_btnPrev.rect = QRect(x, y, kBtnW, rowH);

    // 替换行按钮
    if (m_replaceVisible) {
        int y2 = topMargin + rowH + 6 + 3;
        int rx = width() - rightMargin;
        rx -= kReplBtnW;
        m_btnReplaceAll.rect = QRect(rx, y2, kReplBtnW, rowH);
        rx -= kBtnGap;
        rx -= kReplBtnW;
        m_btnReplace.rect = QRect(rx, y2, kReplBtnW, rowH);
    }
}

SearchBar::ToolButton* SearchBar::hitTest(const QPoint& pos)
{
    if (m_btnPrev.rect.contains(pos)) return &m_btnPrev;
    if (m_btnNext.rect.contains(pos)) return &m_btnNext;
    if (m_btnClose.rect.contains(pos)) return &m_btnClose;
    if (m_replaceVisible) {
        if (m_btnReplace.rect.contains(pos)) return &m_btnReplace;
        if (m_btnReplaceAll.rect.contains(pos)) return &m_btnReplaceAll;
    }
    return nullptr;
}

void SearchBar::mouseMoveEvent(QMouseEvent* event)
{
    updateButtonRects();
    ToolButton* allBtns[] = {&m_btnPrev, &m_btnNext, &m_btnClose, &m_btnReplace, &m_btnReplaceAll};
    bool needUpdate = false;
    for (auto* btn : allBtns) {
        bool h = btn->rect.contains(event->pos());
        if (h != btn->hovered) {
            btn->hovered = h;
            needUpdate = true;
        }
    }
    if (needUpdate) {
        setCursor(hitTest(event->pos()) ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
}

void SearchBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    updateButtonRects();
    auto* btn = hitTest(event->pos());
    if (btn) {
        btn->pressed = true;
        update();
    }
}

void SearchBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    updateButtonRects();
    auto* btn = hitTest(event->pos());

    // Reset all pressed states
    ToolButton* allBtns[] = {&m_btnPrev, &m_btnNext, &m_btnClose, &m_btnReplace, &m_btnReplaceAll};
    for (auto* b : allBtns) b->pressed = false;

    if (btn == &m_btnPrev) emit findPrev(m_findEdit->text());
    else if (btn == &m_btnNext) emit findNext(m_findEdit->text());
    else if (btn == &m_btnClose) hideBar();
    else if (btn == &m_btnReplace) emit replaceNext(m_findEdit->text(), m_replaceEdit->text());
    else if (btn == &m_btnReplaceAll) emit replaceAll(m_findEdit->text(), m_replaceEdit->text());

    update();
}

void SearchBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    updateButtonRects();

    bool dark = m_theme.isDark;

    // 面板背景
    QColor bgColor = dark ? QColor(48, 48, 48, 235) : QColor(255, 255, 255, 245);
    QColor borderColor = dark ? QColor(64, 64, 64, 200) : QColor(200, 200, 200, 200);

    p.setPen(QPen(borderColor, 1));
    p.setBrush(bgColor);
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 5, 5);

    // 按钮样式
    QColor iconColor = dark ? QColor(220, 220, 220) : QColor(80, 80, 80);
    QColor hoverBg = dark ? QColor(255, 255, 255, 30) : QColor(0, 0, 0, 20);
    QColor pressBg = dark ? QColor(255, 255, 255, 50) : QColor(0, 0, 0, 40);

    auto drawBtn = [&](const ToolButton& btn) {
        if (btn.pressed)
            p.fillRect(btn.rect, pressBg);
        else if (btn.hovered)
            p.fillRect(btn.rect, hoverBg);
    };

    auto drawIcon = [&](const ToolButton& btn, auto drawFunc) {
        drawBtn(btn);
        p.setPen(QPen(iconColor, 1.5));
        p.setBrush(Qt::NoBrush);
        drawFunc(btn.rect);
    };

    // Prev button: <
    drawIcon(m_btnPrev, [&](const QRect& r) {
        int cx = r.center().x(), cy = r.center().y();
        QPolygonF arrow;
        arrow << QPointF(cx + 3, cy - 5) << QPointF(cx - 3, cy) << QPointF(cx + 3, cy + 5);
        p.drawPolyline(arrow);
    });

    // Next button: >
    drawIcon(m_btnNext, [&](const QRect& r) {
        int cx = r.center().x(), cy = r.center().y();
        QPolygonF arrow;
        arrow << QPointF(cx - 3, cy - 5) << QPointF(cx + 3, cy) << QPointF(cx - 3, cy + 5);
        p.drawPolyline(arrow);
    });

    // Close button: X
    drawIcon(m_btnClose, [&](const QRect& r) {
        int cx = r.center().x(), cy = r.center().y();
        int s = 4;
        p.drawLine(cx - s, cy - s, cx + s, cy + s);
        p.drawLine(cx + s, cy - s, cx - s, cy + s);
    });

    // 替换行按钮（文字按钮）
    if (m_replaceVisible) {
        QFont btnFont = font();
        btnFont.setPointSize(9);
        p.setFont(btnFont);

        auto drawTextBtn = [&](const ToolButton& btn, const QString& text) {
            drawBtn(btn);
            p.setPen(iconColor);
            p.drawText(btn.rect, Qt::AlignCenter, text);
        };

        drawTextBtn(m_btnReplace, "替换");
        drawTextBtn(m_btnReplaceAll, "全部");
    }
}
