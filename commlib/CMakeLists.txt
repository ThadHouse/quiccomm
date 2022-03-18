set (SOURCES
    src/QuicConnection.c
)

if (APPLE)
set (PLAT_SOURCES src/darwin/HPTimer.c)
elseif (WIN32)
set (PLAT_SOURCES src/win/HPTimer.c)
else()
set (PLAT_SOURCES src/unix/HPTimer.c)
endif()

add_library(commlib STATIC ${SOURCES} ${PLAT_SOURCES})
target_link_libraries(commlib PUBLIC msquic base_link warnings)
target_include_directories(commlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
