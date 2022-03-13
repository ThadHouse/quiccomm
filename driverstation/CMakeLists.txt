set (SOURCES
    driverstation.cpp
    dsevents.cpp
    robotcomms.cpp
)

add_library(driverstation SHARED ${SOURCES})
target_link_libraries(driverstation PRIVATE netcommon)
target_include_directories(driverstation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(driverstation_static STATIC ${SOURCES})
target_link_libraries(driverstation_static PUBLIC netcommon)
target_include_directories(driverstation_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
