#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

class SearchWorker : public QObject {
    Q_OBJECT
public:
    explicit SearchWorker(QObject* parent = nullptr);

public slots:
    void search(const QString& text, const QString& fullText, int requestId);

signals:
    void searchFinished(QVector<QPair<int,int>> matches, int requestId);
};
