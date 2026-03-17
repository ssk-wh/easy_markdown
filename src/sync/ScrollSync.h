#pragma once
#include <QObject>

class EditorWidget;
class PreviewWidget;

class ScrollSync : public QObject {
    Q_OBJECT
public:
    ScrollSync(EditorWidget* editor, PreviewWidget* preview, QObject* parent = nullptr);

    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

private slots:
    void onEditorScrolled();

private:
    EditorWidget* m_editor;
    PreviewWidget* m_preview;
    bool m_enabled = true;
    bool m_syncing = false;
};
