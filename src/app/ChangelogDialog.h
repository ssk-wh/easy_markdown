#pragma once

#include <QDialog>
#include "Theme.h"

class PreviewWidget;

class ChangelogDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChangelogDialog(const Theme& theme, QWidget* parent = nullptr);

private:
    void loadChangelog();
    PreviewWidget* m_previewWidget;
};
