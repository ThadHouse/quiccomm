set (SOURCES
    src/QuicConnection.c
    src/darwin/HPThread.c
)

add_library(commlib STATIC ${SOURCES})
target_link_libraries(commlib PUBLIC msquic base_link warnings)
target_include_directories(commlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
