set (SOURCES
    src/dscomm.c
)

add_library(frcnetcomm_static STATIC ${SOURCES})
target_link_libraries(frcnetcomm_static PUBLIC commlib c11threads)
target_include_directories(frcnetcomm_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(frcnetcomm_static PUBLIC c_std_17)
