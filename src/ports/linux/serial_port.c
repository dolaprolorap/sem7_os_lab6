#include <pty.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <stdbool.h>
#include "serial_port.h"
#include <pthread.h>

#define DAY_DURATION 10
#define PI 3.141592653589793

#define TEMPERATURE_AMPLITUDE 10
#define TEMPERATURE_SHIFT 20

int master, slave;
char slave_name[200];
unsigned long long ticks = 0;

pthread_t serial_thread;
bool pty_is_ready = false;

double get_temperature(void)
{
    double argument = (double) ticks / DAY_DURATION * 2 * PI;
    int random = rand() % 10000;
    double noise = (double) random / 10000 * 0.5;

    return TEMPERATURE_AMPLITUDE * (sin(argument) + 0.1) * noise + TEMPERATURE_SHIFT;
}

void* init_pty(void* arg)
{
    srand(time(NULL));

    if (openpty(&master, &slave, slave_name, NULL, NULL) == -1) {
        perror("Ошибка PTY\n");
        return 1;
    }

    printf("Открыт PTY-slave: %s\n", slave_name);

    char temperature_message[20];

    struct termios tio;
    tcgetattr(slave, &tio);
    cfmakeraw(&tio);
    tcsetattr(slave, TCSANOW, &tio);

    pty_is_ready = true;

    for (;;) {
        ticks++;

        sprintf(temperature_message, "%.5f\n", get_temperature());

        write(master, temperature_message, sizeof(temperature_message));

        sleep(1);
    }

    close(master);
    close(slave);
}

double read_serial()
{
    char buf[100];

    read(slave, buf, sizeof(buf));

    return strtod(buf, NULL);
}

void start_serial(void)
{
    pthread_create(&serial_thread, NULL, init_pty, NULL);

    while (!pty_is_ready) {
        sleep(1);
    }
}
