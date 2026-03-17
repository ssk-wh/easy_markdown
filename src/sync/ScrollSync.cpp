#include "ScrollSync.h"
#include "EditorWidget.h"
#include "PreviewWidget.h"
#include <QScrollBar>

ScrollSync::ScrollSync(EditorWidget* editor, PreviewWidget* preview, QObject* parent)
    : QObject(parent)
    , m_editor(editor)
    , m_preview(preview)
{
    connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &ScrollSync::onEditorScrolled);
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
