set (SOURCES
    main.c
)

add_executable(quicclient ${SOURCES})
target_link_libraries(quicclient commlib)
