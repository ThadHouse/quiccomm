set (SOURCES
    QuicApi.cpp
)

add_library(netcommon STATIC ${SOURCES})
target_link_libraries(netcommon PUBLIC msquic inc warnings)
target_include_directories(netcommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
