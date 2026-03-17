#pragma once
#include <QObject>

class EditorLayout : public QObject {
    Q_OBJECT
public:
    explicit EditorLayout(QObject* p = nullptr);
};
