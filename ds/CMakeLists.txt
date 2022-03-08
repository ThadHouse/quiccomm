set (SOURCES
    frcds.cpp
)

add_library(frcds SHARED ${SOURCES})
target_link_libraries(frcds PRIVATE netcommon msquic)
target_include_directories(frcds PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(frcds_static STATIC ${SOURCES})
target_link_libraries(frcds_static PUBLIC netcommon msquic)
target_include_directories(frcds_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
