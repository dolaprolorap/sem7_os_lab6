cmake_minimum_required(VERSION 3.28)
project(lab6)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(INCLUDES include)

    set(WIN_SOURCE
        src/main.cpp
    )

    set(EXTRA_LIBS
        kernel32
    )

    add_executable(lab6 ${WIN_SOURCE})

    target_link_libraries(lab6 PRIVATE ${EXTRA_LIBS})

    target_include_directories(lab6 PRIVATE ${INCLUDES})
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
    find_package(Qt5 REQUIRED COMPONENTS Network)

    set(INCLUDES 
        include
        /usr/include/qwt
    )

    set(LINUX_SOURCE
        src/main.cpp
    )

    add_executable(lab6 ${LINUX_SOURCE})

    target_link_libraries(lab6 
        Qt5::Widgets
        Qt5::Network
        qwt-qt5
        m
    )

    target_include_directories(lab6 PRIVATE ${INCLUDES})
endif()

target_compile_options(lab6 PRIVATE -std=c11 -fPIC)
