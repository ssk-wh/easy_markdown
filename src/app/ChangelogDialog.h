#pragma once

#include <QDialog>

class QTextEdit;

class ChangelogDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChangelogDialog(QWidget* parent = nullptr);

private:
    void loadChangelog();
    QTextEdit* m_textEdit;
};
