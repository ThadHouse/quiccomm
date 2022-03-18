set (SOURCES
    main.c
)

add_executable(quicserver ${SOURCES})
target_link_libraries(quicserver frccert commlib)
