cmake_minimum_required(VERSION 3.28)
project(lab4)

set(INCLUDES include)

if(WIN32)
    set(WIN_SOURCE
        src/main.c
        src/ports/win/logger.c
        src/ports/win/serial_port.c
        src/ports/win/utils.c
    )

    set(EXTRA_LIBS
        kernel32
    )

    add_executable(lab4 ${WIN_SOURCE})

    target_link_libraries(lab4 PRIVATE ${EXTRA_LIBS})
else()
    set(LINUX_SOURCE
        src/main.c
        src/ports/linux/logger.c
        src/ports/linux/serial_port.c
        src/ports/linux/utils.c
    )

    add_executable(lab4 ${LINUX_SOURCE})
endif()

target_include_directories(lab4 PRIVATE ${INCLUDES})

target_link_libraries(lab4 PRIVATE m)

target_compile_options(lab4 PRIVATE -std=c11)
