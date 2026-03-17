#pragma once
#include <QWidget>

class QLineEdit;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class EditorWidget;

class SearchBar : public QWidget {
    Q_OBJECT
public:
    explicit SearchBar(EditorWidget* parent);

    void showSearch();      // Ctrl+F
    void showReplace();     // Ctrl+H
    void hideBar();

    QString searchText() const;

signals:
    void findNext(const QString& text);
    void findPrev(const QString& text);
    void replaceNext(const QString& find, const QString& replace);
    void replaceAll(const QString& find, const QString& replace);
    void searchTextChanged(const QString& text);
    void closed();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QLineEdit* m_findEdit;
    QLineEdit* m_replaceEdit;
    QWidget* m_replaceRow;
    QPushButton* m_findNextBtn;
    QPushButton* m_findPrevBtn;
    QPushButton* m_replaceBtn;
    QPushButton* m_replaceAllBtn;
    QPushButton* m_closeBtn;
};
