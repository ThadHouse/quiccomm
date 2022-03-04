set (SOURCES
    netcomm.cpp
    remoteconnection.cpp
)

add_library(frcnetcomm SHARED ${SOURCES})
target_link_libraries(frcnetcomm PRIVATE netcommon)
target_include_directories(frcnetcomm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
