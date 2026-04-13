//
// Spec: specs/横切关注点/80-字体系统.md
// Invariants enforced here: INV-1 (single source of truth), INV-2/6 (visual alignment),
//                           INV-9 (mono delta), INV-10 (balance editor font size)
//
// 默认字号/字体族的唯一数据源。禁止在任何其他文件中硬编码字号数字（除 < 10 的
// 局部小字号如按钮次要文字，且必须加注释说明）。
//

#pragma once

#include <QFont>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QPaintDevice>
#include <cmath>
#include <limits>

namespace font_defaults {

// 基础字号（预览正文字号；编辑器字号由 balanceEditorFontSize 动态补偿）
constexpr int kDefaultBaseFontSizePt = 12;

// 等宽字号（代码块、内联代码默认值）
constexpr int kMonoFontSizePt = 9;

// 等宽字号相对基础字号的派生差值（INV-9）
constexpr int kMonoDelta = kMonoFontSizePt - kDefaultBaseFontSizePt;  // = -3

// 字体族
constexpr const char* kEditorFontFamily = "Courier New";
constexpr const char* kPreviewFontFamily = "Segoe UI";
constexpr const char* kMonoFontFamily = "Consolas";

// 编辑器默认字体（INV-3）
// 注意：此函数返回的 pointSize 是 base + delta；MainWindow::applyFontSize 必须
// 再调用 balanceEditorFontSize 做视觉补偿（INV-10）
inline QFont defaultEditorFont(int sizeDelta = 0)
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(kDefaultBaseFontSizePt + sizeDelta);
    font.setStyleHint(QFont::Monospace);
    return font;
}

// 预览默认字体（基础正文字体）
inline QFont defaultPreviewFont(int sizeDelta = 0)
{
    return QFont(kPreviewFontFamily, kDefaultBaseFontSizePt + sizeDelta);
}

// 预览代码块/内联代码等宽字体（INV-9：必须随 sizeDelta 缩放）
inline QFont defaultMonoFont(int sizeDelta = 0)
{
    QFont font(kMonoFontFamily, kDefaultBaseFontSizePt + sizeDelta + kMonoDelta);
    font.setStyleHint(QFont::Monospace);
    return font;
}

// 视觉对齐：调整编辑器字号让 xHeight 接近预览字体（INV-2 / INV-10）
//
// 算法：在 [previewFont.pointSize() - 2, previewFont.pointSize() + 2] 窗口内
// 搜索候选 pointSize，挑选使其 QFontMetricsF::xHeight() 与预览最接近的值。
//
// 若 device 为 null（如单元测试场景），退化到原 editor 字号不做补偿。
inline QFont balanceEditorFontSize(QFont editor, const QFont& preview, QPaintDevice* device)
{
    if (!device) {
        return editor;
    }
    QFontMetricsF prevFm(preview, device);
    const qreal target = prevFm.xHeight();

    int basePt = preview.pointSize();
    if (basePt <= 0) {
        // pointSize 未设置（用了 pointSizeF 或 pixelSize），fallback 不补偿
        return editor;
    }

    int bestPt = editor.pointSize() > 0 ? editor.pointSize() : basePt;
    qreal bestDiff = std::numeric_limits<qreal>::max();

    for (int pt = basePt - 2; pt <= basePt + 2; ++pt) {
        if (pt <= 0) continue;
        QFont trial = editor;
        trial.setPointSize(pt);
        QFontMetricsF fm(trial, device);
        qreal diff = std::abs(fm.xHeight() - target);
        if (diff < bestDiff) {
            bestDiff = diff;
            bestPt = pt;
        }
    }

    editor.setPointSize(bestPt);
    return editor;
}

} // namespace font_defaults
