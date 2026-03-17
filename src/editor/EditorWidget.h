#pragma once
#include <QAbstractScrollArea>

class EditorWidget : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit EditorWidget(QWidget* p = nullptr);
};
