add_executable(netcommapp main.cpp)
target_link_libraries(netcommapp frcnetcomm_static wpiutil inc warnings)
