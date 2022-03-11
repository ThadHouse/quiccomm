set (SOURCES
    src/QuicApi.cpp
    src/QuicConnection.cpp
    src/tags/TagManager.cpp
    src/tags/JoystickData.cpp
)

add_library(netcommon STATIC ${SOURCES})
target_link_libraries(netcommon PUBLIC msquic wpiutil base_link inc warnings)
target_include_directories(netcommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
