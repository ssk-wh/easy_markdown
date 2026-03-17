#pragma once

struct TextPosition {
    int line = 0;
    int column = 0;

    bool operator==(const TextPosition& other) const;
    bool operator!=(const TextPosition& other) const;
    bool operator<(const TextPosition& other) const;
    bool operator<=(const TextPosition& other) const;
    bool operator>(const TextPosition& other) const;
    bool operator>=(const TextPosition& other) const;
};

struct SelectionRange {
    TextPosition anchor;
    TextPosition cursor;

    bool isEmpty() const;
    bool isForward() const;
    TextPosition start() const;
    TextPosition end() const;
};

class Selection {
public:
    Selection();

    TextPosition cursorPosition() const;
    void setCursorPosition(TextPosition pos);

    SelectionRange range() const;
    bool hasSelection() const;
    void setSelection(TextPosition anchor, TextPosition cursor);
    void clearSelection();

    void extendSelection(TextPosition newCursor);

    int preferredColumn() const;
    void setPreferredColumn(int col);
    void resetPreferredColumn();

private:
    SelectionRange m_range;
    int m_preferredColumn = -1;
};
