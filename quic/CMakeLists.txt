set (SOURCES
    src/QuicConnection.c
)

add_library(quicconn SHARED ${SOURCES})
target_link_libraries(quicconn PUBLIC msquic base_link warnings)
target_include_directories(quicconn PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
