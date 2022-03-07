project(quiccomm)
cmake_minimum_required(VERSION 3.16)

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
option(WITH_JAVA "Include java and JNI in the build" OFF)
option(WITH_CSCORE "Build cscore (needs OpenCV)" OFF)
option(WITH_WPIMATH "Build wpimath" OFF)
option(WITH_WPILIB "Build hal, wpilibc/j, and myRobot (needs OpenCV)" OFF)
option(WITH_OLD_COMMANDS "Build old commands" OFF)
option(WITH_EXAMPLES "Build examples" OFF)
option(WITH_TESTS "Build unit tests (requires internet connection)" OFF)
option(WITH_GUI "Build GUI items" OFF)
option(WITH_SIMULATION_MODULES "Build simulation modules" OFF)
add_subdirectory(allwpilib)

# Configure and build msquic dependency.
set(QUIC_TLS "openssl")
set(QUIC_ENABLE_LOGGING ON CACHE BOOL "Enable MsQuic logging")
add_subdirectory(msquic)
target_compile_features(inc INTERFACE cxx_std_17)

add_subdirectory(common)
add_subdirectory(ds)
add_subdirectory(dsapp)
add_subdirectory(netcomm)
add_subdirectory(netcommapp)
