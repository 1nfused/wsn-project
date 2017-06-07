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

/* COMMAND */
#define SET_START_M_CAST		"Z:MULT"
#define GET_HEART_BEAT			"Z:H?"
#define GET_TEMP_MIN			"Z:S:T:MIN?"
#define GET_TEMP_MAX			"Z:S:T:MAX?"
#define GET_TEMP_AVG			"Z:S:T:AVG?"
#define GET_VIB_MIN				"Z:S:VI:MIN?"
#define GET_VIB_MAX				"Z:S:VI:MAX?"
#define GET_VIB_AVG				"Z:S:VI:AVG?"
#define GET_VOLT_MIN			"Z:S:VO:MIN?"
#define GET_VOLT_MAX			"Z:S:VO:MAX?"
#define GET_VOLT_AVG			"Z:S:VO:AVG?"


long get_temperature();
int abs(int v);
int get_normed_vibr();

#endif