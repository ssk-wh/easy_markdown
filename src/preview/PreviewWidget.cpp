#include "PreviewWidget.h"
#include "PreviewLayout.h"
#include "PreviewPainter.h"
#include "ImageCache.h"

#include <QPainter>
#include <QScrollBar>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
#include <QMenu>

PreviewWidget::PreviewWidget(QWidget* parent)
    : QAbstractScrollArea(parent)
{
    m_layout = new PreviewLayout();
    m_layout->setFont(font());

    m_painter = new PreviewPainter();

    m_imageCache = new ImageCache(this);

    viewport()->setAutoFillBackground(true);
    QPalette pal = viewport()->palette();
    pal.setColor(QPalette::Window, Qt::white);
    viewport()->setPalette(pal);

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    viewport()->setCursor(Qt::IBeamCursor);
    viewport()->setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
}

PreviewWidget::~PreviewWidget()
{
    delete m_layout;
    delete m_painter;
    // m_imageCache is owned by QObject parent
}

PreviewLayout* PreviewWidget::previewLayout() const
{
    return m_layout;
}

void PreviewWidget::updateAst(std::shared_ptr<AstNode> root)
{
    m_currentAst = std::move(root);

    qreal contentWidth = m_wordWrap ? (viewport()->width() - 40) : 10000;
    if (contentWidth < 100) contentWidth = 100;

    m_layout->setViewportWidth(contentWidth);
    m_layout->buildFromAst(m_currentAst);

    m_plainText = extractPlainText();
    m_selStart = m_selEnd = -1;

    updateScrollBars();
    viewport()->update();
}

void PreviewWidget::rebuildLayout()
{
    if (!m_currentAst) return;
    qreal contentWidth = m_wordWrap ? (viewport()->width() - 40) : 10000;
    if (contentWidth < 100) contentWidth = 100;
    m_layout->setViewportWidth(contentWidth);
    m_layout->buildFromAst(m_currentAst);
    updateScrollBars();
}

void PreviewWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(viewport());
    painter.fillRect(viewport()->rect(), m_theme.previewBg);

    if (!m_currentAst) return;

    // 检测 DPI 变化（切换屏幕时用新设备度量重建布局）
    qreal currentDpr = viewport()->devicePixelRatioF();
    if (!qFuzzyCompare(currentDpr, m_lastDevicePixelRatio)) {
        m_lastDevicePixelRatio = currentDpr;
        m_layout->updateMetrics(viewport());
        rebuildLayout();
    }

    qreal scrollY = verticalScrollBar()->value();
    qreal vpHeight = viewport()->height();
    qreal vpWidth = viewport()->width();

    // Apply 20px horizontal padding, minus horizontal scroll
    qreal scrollXVal = m_wordWrap ? 0 : horizontalScrollBar()->value();
    painter.translate(20 - scrollXVal, 0);

    m_painter->setSelection(m_selStart, m_selEnd);
    qreal contentWidth = m_wordWrap ? (vpWidth - 40) : 10000;
    m_painter->paint(&painter, m_layout->rootBlock(), scrollY, vpHeight, contentWidth);
}

void PreviewWidget::resizeEvent(QResizeEvent* event)
{
    QAbstractScrollArea::resizeEvent(event);
    rebuildLayout();
}

void PreviewWidget::scrollContentsBy(int /*dx*/, int /*dy*/)
{
    viewport()->update();
}

void PreviewWidget::updateScrollBars()
{
    qreal totalH = m_layout->totalHeight();
    qreal vpH = viewport()->height();
    int maxScroll = qMax(0, static_cast<int>(totalH - vpH));
    verticalScrollBar()->setRange(0, maxScroll);
    verticalScrollBar()->setPageStep(static_cast<int>(vpH));
    verticalScrollBar()->setSingleStep(20);

    if (!m_wordWrap) {
        qreal totalW = m_layout->rootBlock().bounds.width() + 40;
        qreal vpW = viewport()->width();
        horizontalScrollBar()->setRange(0, qMax(0, static_cast<int>(totalW - vpW)));
        horizontalScrollBar()->setPageStep(static_cast<int>(vpW));
        horizontalScrollBar()->setSingleStep(20);
    } else {
        horizontalScrollBar()->setRange(0, 0);
    }
}

void PreviewWidget::setTheme(const Theme& theme)
{
    m_theme = theme;
    m_painter->setTheme(theme);
    m_layout->setTheme(theme);

    // 重建 layout 以更新 InlineRun 中的主题色
    if (m_currentAst) {
        m_layout->buildFromAst(m_currentAst);
        updateScrollBars();
    }

    QPalette pal = viewport()->palette();
    pal.setColor(QPalette::Window, theme.previewBg);
    viewport()->setPalette(pal);

    viewport()->update();
}

void PreviewWidget::setWordWrap(bool enabled)
{
    m_wordWrap = enabled;

    if (enabled) {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

    if (m_currentAst) {
        qreal contentWidth = m_wordWrap ? (viewport()->width() - 40) : 10000;
        if (contentWidth < 100) contentWidth = 100;
        m_layout->setViewportWidth(contentWidth);
        m_layout->buildFromAst(m_currentAst);
        m_plainText = extractPlainText();
        updateScrollBars();
    }
    viewport()->update();
}

void PreviewWidget::scrollToSourceLine(int line)
{
    qreal y = m_layout->sourceLineToY(line);
    verticalScrollBar()->setValue(static_cast<int>(y));
}

void PreviewWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_selecting = true;
        // segment rects 在 painter translate 后的视口坐标系，鼠标也转到同一坐标系
        qreal scrollXVal = m_wordWrap ? 0 : horizontalScrollBar()->value();
        QPointF pt(event->pos().x() - 20 + scrollXVal, event->pos().y());
        m_selStart = m_selEnd = textIndexAtPoint(pt);
        viewport()->update();
    }
    QAbstractScrollArea::mousePressEvent(event);
}

void PreviewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_selecting && (event->buttons() & Qt::LeftButton)) {
        qreal scrollXVal = m_wordWrap ? 0 : horizontalScrollBar()->value();
        QPointF pt(event->pos().x() - 20 + scrollXVal, event->pos().y());
        m_selEnd = textIndexAtPoint(pt);
        viewport()->update();
    }
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    m_selecting = false;
    QAbstractScrollArea::mouseReleaseEvent(event);
}

void PreviewWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !m_plainText.isEmpty()) {
        qreal scrollXVal = m_wordWrap ? 0 : horizontalScrollBar()->value();
        QPointF pt(event->pos().x() - 20 + scrollXVal, event->pos().y());
        int idx = textIndexAtPoint(pt);

        if (idx >= 0 && idx < m_plainText.length()) {
            auto isWordChar = [](QChar c) {
                return c.isLetterOrNumber() || c == QLatin1Char('_');
            };

            if (isWordChar(m_plainText.at(idx))) {
                int start = idx;
                while (start > 0 && isWordChar(m_plainText.at(start - 1)))
                    --start;
                int end = idx + 1;
                while (end < m_plainText.length() && isWordChar(m_plainText.at(end)))
                    ++end;
                m_selStart = start;
                m_selEnd = end;
            } else {
                m_selStart = idx;
                m_selEnd = idx + 1;
            }

            m_selecting = false;
            viewport()->update();
        }
    }
}

void PreviewWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->matches(QKeySequence::Copy)) {
        copySelection();
        return;
    }
    if (event->matches(QKeySequence::SelectAll)) {
        m_selStart = 0;
        m_selEnd = m_plainText.length();
        viewport()->update();
        return;
    }
    QAbstractScrollArea::keyPressEvent(event);
}

void PreviewWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    QAction* copyAct = menu.addAction(tr("Copy"), this, &PreviewWidget::copySelection, QKeySequence::Copy);
    int start = qMin(m_selStart, m_selEnd);
    int end = qMax(m_selStart, m_selEnd);
    copyAct->setEnabled(start >= 0 && end > start);

    QAction* selectAllAct = menu.addAction(tr("Select All"), [this]() {
        m_selStart = 0;
        m_selEnd = m_plainText.length();
        viewport()->update();
    }, QKeySequence::SelectAll);
    Q_UNUSED(selectAllAct);

    menu.exec(event->globalPos());
}

void PreviewWidget::copySelection()
{
    if (m_selStart < 0 || m_selEnd < 0) return;
    int start = qMin(m_selStart, m_selEnd);
    int end = qMax(m_selStart, m_selEnd);
    if (start == end) return;

    QString sel = m_plainText.mid(start, end - start);
    QApplication::clipboard()->setText(sel);
}

QString PreviewWidget::extractPlainText() const
{
    QString text;
    extractBlockText(m_layout->rootBlock(), text);
    return text;
}

void PreviewWidget::extractBlockText(const LayoutBlock& block, QString& out) const
{
    // Inline text - 匹配 paintInlineRuns 的计数：所有 run.text + 无条件分隔换行
    if (!block.inlineRuns.empty()) {
        for (const auto& run : block.inlineRuns) {
            out += run.text;
        }
        out += '\n';
    }

    // Code block - 匹配 paintBlock 的逐行计数，跳过 split 产生的尾部空元素
    if (!block.codeText.isEmpty()) {
        const QStringList lines = block.codeText.split('\n');
        for (int i = 0; i < lines.size(); ++i) {
            if (i == lines.size() - 1 && lines[i].isEmpty())
                break;
            out += lines[i];
            out += '\n';
        }
    }

    // Recurse into children
    for (const auto& child : block.children) {
        extractBlockText(child, out);
    }
}

// 在段内使用字体度量逐字精确定位字符索引
static int hitTestSegment(const TextSegment& seg, qreal relX)
{
    if (seg.text.isEmpty()) return seg.charStart;
    QFontMetricsF fm(seg.font);
    for (int i = 0; i < seg.text.length(); ++i) {
        qreal w = fm.horizontalAdvance(seg.text.left(i + 1));
        if (relX < w) {
            qreal prevW = (i > 0) ? fm.horizontalAdvance(seg.text.left(i)) : 0;
            return seg.charStart + ((relX - prevW < w - relX) ? i : i + 1);
        }
    }
    return seg.charStart + seg.charLen;
}

int PreviewWidget::textIndexAtPoint(const QPointF& point) const
{
    const auto& segments = m_painter->textSegments();
    if (segments.isEmpty())
        return 0;

    int closest = 0;
    qreal closestDist = std::numeric_limits<qreal>::max();

    for (const auto& seg : segments) {
        if (seg.rect.contains(point)) {
            return hitTestSegment(seg, point.x() - seg.rect.x());
        }

        // 计算点到矩形的 2D 距离（解决表格单元格间隙定位问题）
        qreal dx = 0, dy = 0;
        if (point.x() < seg.rect.left())
            dx = seg.rect.left() - point.x();
        else if (point.x() > seg.rect.right())
            dx = point.x() - seg.rect.right();
        if (point.y() < seg.rect.top())
            dy = seg.rect.top() - point.y();
        else if (point.y() > seg.rect.bottom())
            dy = point.y() - seg.rect.bottom();

        qreal dist = dy * dy + dx * dx;

        if (dist < closestDist) {
            closestDist = dist;
            if (point.x() >= seg.rect.right())
                closest = seg.charStart + seg.charLen;
            else if (point.x() <= seg.rect.left())
                closest = seg.charStart;
            else
                closest = hitTestSegment(seg, point.x() - seg.rect.x());
        }
    }

    return closest;
}
