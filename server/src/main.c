#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "serial_port.h"
#include "logger.h"
#include "utils.h"
#include "repository.h"
#include <stdio.h>
#include "server.h"

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    start_serial();

    init_logger();

    init_db();

    start_server();

    start_hourly_logger();
    start_daily_logger();

    start_clear_logger();
    start_clear_hourly_logger();
    start_clear_daily_logger();

    for (int i = 0; i < 100; i++)
    {
        double temperature = read_serial();

        log_temperature(temperature);

        sleep_(1);
    }

    return 0;
}
