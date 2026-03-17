#include <gtest/gtest.h>
#include "Selection.h"

TEST(TextPosition, Comparison) {
    TextPosition a{0, 5}, b{1, 0}, c{0, 5};
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a == c);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= c);
    EXPECT_TRUE(a >= c);
    EXPECT_FALSE(a != c);
}

TEST(SelectionRange, Empty) {
    SelectionRange r{{1, 2}, {1, 2}};
    EXPECT_TRUE(r.isEmpty());
}

TEST(SelectionRange, Forward) {
    SelectionRange r{{0, 0}, {1, 5}};
    EXPECT_TRUE(r.isForward());
    EXPECT_EQ(r.start(), (TextPosition{0, 0}));
    EXPECT_EQ(r.end(), (TextPosition{1, 5}));
}

TEST(SelectionRange, Backward) {
    SelectionRange r{{1, 5}, {0, 0}};
    EXPECT_FALSE(r.isForward());
    EXPECT_EQ(r.start(), (TextPosition{0, 0}));
    EXPECT_EQ(r.end(), (TextPosition{1, 5}));
}

TEST(Selection, InitialState) {
    Selection sel;
    EXPECT_EQ(sel.cursorPosition(), (TextPosition{0, 0}));
    EXPECT_FALSE(sel.hasSelection());
}

TEST(Selection, SetCursorClearsSelection) {
    Selection sel;
    sel.setSelection({0, 0}, {1, 5});
    EXPECT_TRUE(sel.hasSelection());
    sel.setCursorPosition({2, 0});
    EXPECT_FALSE(sel.hasSelection());
    EXPECT_EQ(sel.cursorPosition(), (TextPosition{2, 0}));
}

TEST(Selection, ExtendSelection) {
    Selection sel;
    sel.setCursorPosition({1, 0});
    sel.extendSelection({2, 5});
    EXPECT_TRUE(sel.hasSelection());
    EXPECT_EQ(sel.range().anchor, (TextPosition{1, 0}));
    EXPECT_EQ(sel.range().cursor, (TextPosition{2, 5}));
}

TEST(Selection, ClearSelection) {
    Selection sel;
    sel.setSelection({0, 0}, {3, 0});
    sel.clearSelection();
    EXPECT_FALSE(sel.hasSelection());
    EXPECT_EQ(sel.cursorPosition(), (TextPosition{3, 0}));
}

TEST(Selection, PreferredColumn) {
    Selection sel;
    EXPECT_EQ(sel.preferredColumn(), -1);
    sel.setPreferredColumn(10);
    EXPECT_EQ(sel.preferredColumn(), 10);
    sel.resetPreferredColumn();
    EXPECT_EQ(sel.preferredColumn(), -1);
}
