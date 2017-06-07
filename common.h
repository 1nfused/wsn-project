#ifndef COMMON_H
#define COMMON_H

#include "dev/tmp102.h"
#include "dev/adxl345.h"
#include "dev/i2cmaster.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "apps/shell/shell.h"
#include "dev/uart0.h"

// Comunication specific data
#define MAX_CMD_LENGTH			100
#define MCAST_SINK_UDP_PORT 	3001 /* Host byte order */
#define UNICAST_CHANNEL			110
#define UDP_CLIENT_PORT 		8765
#define UDP_SERVER_PORT 		5678
#define MAX_PAY_LOAD			120

long get_temperature();
int abs(int v);
int get_normed_vibr();

#endif