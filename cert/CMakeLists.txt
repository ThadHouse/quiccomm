set (SOURCES
    src/frccert.c
)

add_library(frccert STATIC ${SOURCES})
target_link_libraries(frccert PUBLIC warnings)
target_include_directories(frccert PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
