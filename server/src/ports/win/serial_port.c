#include "serial_port.h"
#include "utils.h"
#include <windows.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define SERIAL_INPUT "\\\\.\\COM3"
#define SERIAL_OUTPUT "\\\\.\\COM4"

#define DAY_DURATION 10
#define PI 3.141592653589793

#define TEMPERATURE_AMPLITUDE 10
#define TEMPERATURE_SHIFT 20

unsigned long long ticks = 0;
bool com_inited = false;

double get_temperature(void)
{
    double argument = (double) ticks / DAY_DURATION * 2 * PI;
    int random = rand() % 10000;
    double noise = (double) random / 10000 * 0.5;

    return TEMPERATURE_AMPLITUDE * (sin(argument) + 0.1) * noise + TEMPERATURE_SHIFT;
}

void init_com(void)
{
    srand(time(NULL));

    HANDLE *handle = CreateFileA(
        SERIAL_INPUT,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    printf("Открыт COM-порт");
    
    char temperature_message[20];

    com_inited = true;

    for (;;) {
        ticks++;

        int len = sprintf(temperature_message, "%.5f\n", get_temperature());

        WriteFile(handle, temperature_message, len, NULL, NULL);

        sleep_(1);
    }

    CloseHandle(handle);
}

double read_serial()
{
    char buf[100];
    int bytes_read;
    
    HANDLE *handle = CreateFileA(
        SERIAL_OUTPUT,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    
    ReadFile(
        handle,
        buf,
        sizeof(buf),
        &bytes_read,
        NULL
    );

    CloseHandle(handle);

    return strtod(buf, NULL);
}

void start_serial(void)
{
    _beginthreadex(NULL, 0, init_com, NULL, 0, NULL);

    while (!com_inited) {
        sleep_(1);
    }
}
