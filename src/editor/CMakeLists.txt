add_library(easy_editor STATIC
    EditorLayout.h EditorLayout.cpp
    EditorWidget.h EditorWidget.cpp
    EditorPainter.h EditorPainter.cpp
    EditorInput.h EditorInput.cpp
    GutterRenderer.h GutterRenderer.cpp
    BandBuffer.h BandBuffer.cpp
    SyntaxHighlighter.h SyntaxHighlighter.cpp
    SearchBar.h SearchBar.cpp
)
target_include_directories(easy_editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(easy_editor PUBLIC easy_core Qt5::Widgets)
