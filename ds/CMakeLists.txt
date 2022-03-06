set (SOURCES
    entry.cpp
    frcds.cpp
)

add_library(frcds SHARED ${SOURCES})
target_link_libraries(frcds PRIVATE netcommon)
target_include_directories(frcds PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
