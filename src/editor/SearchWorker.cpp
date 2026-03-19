#include "SearchWorker.h"

SearchWorker::SearchWorker(QObject* parent)
    : QObject(parent)
{
}

void SearchWorker::search(const QString& text, const QString& fullText, int requestId)
{
    QVector<QPair<int,int>> matches;

    if (!text.isEmpty()) {
        int pos = 0;
        int len = text.length();
        while ((pos = fullText.indexOf(text, pos, Qt::CaseInsensitive)) != -1) {
            matches.append({pos, len});
            pos += len;
        }
    }

    emit searchFinished(matches, requestId);
}
