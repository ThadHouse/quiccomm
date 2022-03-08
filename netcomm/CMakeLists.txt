set (SOURCES
    netcomm.cpp
)

add_library(frcnetcomm SHARED ${SOURCES})
target_link_libraries(frcnetcomm PRIVATE netcommon msquic)
target_include_directories(frcnetcomm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(frcnetcomm_static STATIC ${SOURCES})
target_link_libraries(frcnetcomm_static PUBLIC netcommon msquic)
target_include_directories(frcnetcomm_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
