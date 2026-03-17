#pragma once

#include <QString>
#include <QColor>
#include <vector>

class CodeBlockRenderer {
public:
    struct HighlightedSegment {
        QString text;
        QColor color;
        bool bold = false;
    };

    using HighlightedLine = std::vector<HighlightedSegment>;

    CodeBlockRenderer();
    ~CodeBlockRenderer();

    std::vector<HighlightedLine> highlight(const QString& code, const QString& language);
};
