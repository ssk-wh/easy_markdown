#include "ScrollSync.h"
#include "EditorWidget.h"
#include "EditorLayout.h"
#include "PreviewWidget.h"
#include "PreviewLayout.h"
#include <QScrollBar>

ScrollSync::ScrollSync(EditorWidget* editor, PreviewWidget* preview, QObject* parent)
    : QObject(parent)
    , m_editor(editor)
    , m_preview(preview)
{
    connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &ScrollSync::onEditorScrolled);
    connect(m_preview->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &ScrollSync::onPreviewScrolled);
}

void ScrollSync::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

void ScrollSync::onEditorScrolled()
{
    if (!m_enabled || m_syncing)
        return;

    m_syncing = true;
    int firstLine = m_editor->firstVisibleLine();
    m_preview->scrollToSourceLine(firstLine);
    m_syncing = false;
}

void ScrollSync::onPreviewScrolled()
{
    if (!m_enabled || m_syncing)
        return;

    m_syncing = true;
    qreal previewScrollY = m_preview->verticalScrollBar()->value();
    int sourceLine = m_preview->previewLayout()->yToSourceLine(previewScrollY);
    qreal editorY = m_editor->editorLayout()->lineY(sourceLine);
    m_editor->verticalScrollBar()->setValue(static_cast<int>(editorY));
    m_syncing = false;
}
