cmake_minimum_required(VERSION 3.28)
project(lab6)

set(INCLUDES include)

if(WIN32)
    set(WIN_SOURCE
        src/main.c
        src/ports/win/logger.c
        src/ports/win/serial_port.c
        src/ports/win/utils.c
        src/ports/win/server.c
        src/sqlite3.c
        src/repository.c
    )

    set(EXTRA_LIBS
        kernel32
        ws2_32
    )

    add_executable(lab6 ${WIN_SOURCE})

    target_link_libraries(lab6 PRIVATE ${EXTRA_LIBS})
else()
    set(LINUX_SOURCE
        src/main.c
        src/ports/linux/logger.c
        src/ports/linux/serial_port.c
        src/ports/linux/utils.c
        src/ports/linux/server.c
        src/sqlite3.c
        src/repository.c
    )

    add_executable(lab6 ${LINUX_SOURCE})
endif()

target_include_directories(lab6 PRIVATE ${INCLUDES})

target_link_libraries(lab6 PRIVATE m)

target_compile_options(lab6 PRIVATE -std=c11)
