set (SOURCES
    entry.cpp
)

add_library(frcds SHARED ${SOURCES})
target_link_libraries(frcds PRIVATE netcommon)
