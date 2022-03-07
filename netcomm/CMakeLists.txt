set (SOURCES
    netcomm.cpp
)

add_library(frcnetcomm SHARED ${SOURCES})
target_link_libraries(frcnetcomm PRIVATE netcommon msquic)
target_include_directories(frcnetcomm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
