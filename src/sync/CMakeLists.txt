add_library(easy_sync STATIC
    ScrollSync.h ScrollSync.cpp
)
target_include_directories(easy_sync PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(easy_sync PUBLIC easy_editor easy_preview)
