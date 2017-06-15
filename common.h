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
#define MAX_CMD_LENGTH				100
#define MCAST_SINK_UDP_PORT 		3001 /* Host byte order */
#define UNICAST_CHANNEL				110
#define UDP_CLIENT_PORT 			8765
#define UDP_SERVER_PORT 			5678
#define MAX_PAY_LOAD				120
#define DELAY_CHECK_SENSORS_VALUE 	5

// Result buffers
#define NODE_RES_FORMAT				"%d-%d"
#define NODE_ALRM_T_FORMAT			"A_T-%d-%d"
#define NODE_ALRM_V_FORMAT			"A_V-%d-%d"

/* COMMAND */
#define GET_USAGE				"Z:USAGE"
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

//Thresholds
#define GET_UPDATE_CLK			"Z:U:THR:CLK?"
#define SET_UPDATE_CLK			"Z:U:THR:CLK"

#define GET_TEMP_THR			"Z:S:T:THR?"
#define SET_TEMP_THR			"Z:S:T:THR"
#define GET_VIB_THR				"Z:S:VI:THR?"
#define SET_VIB_THR				"Z:S:VI:THR"
#define GET_VOLT_THR			"Z:S:VO:THR?"
#define SET_VOLT_THR			"Z:S:VO:THR"

#endif