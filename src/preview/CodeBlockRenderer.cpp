#include "CodeBlockRenderer.h"

CodeBlockRenderer::CodeBlockRenderer() = default;
CodeBlockRenderer::~CodeBlockRenderer() = default;

std::vector<CodeBlockRenderer::HighlightedLine>
CodeBlockRenderer::highlight(const QString& code, const QString& /*language*/)
{
    std::vector<HighlightedLine> result;
    const QStringList lines = code.split('\n');

    for (const auto& line : lines) {
        HighlightedLine hl;
        HighlightedSegment seg;
        seg.text = line;
        seg.color = QColor("#333333");
        seg.bold = false;
        hl.push_back(std::move(seg));
        result.push_back(std::move(hl));
    }

    return result;
}
