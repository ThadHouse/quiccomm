set (SOURCES
    frcds.cpp
)

add_library(frcds SHARED ${SOURCES})
target_link_libraries(frcds PRIVATE netcommon msquic)
target_include_directories(frcds PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
