#pragma once
#include <QObject>

class Document : public QObject {
    Q_OBJECT
public:
    explicit Document(QObject* parent = nullptr);
};
