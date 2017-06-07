#ifndef V_ROOT_H
#define V_ROOT_H

#define MAX_PAYLOAD_LEN 120

#define SEND_INTERVAL CLOCK_SECOND /* clock ticks */
#define ITERATIONS 100 /* messages */
#define DEBUG DEBUG_PRINT
#define SEND_INTERVAL CLOCK_SECOND /* clock ticks */
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
#define DELAY_CHECK_SENSORS_VALUE 3

/* Start sending messages START_DELAY secs after we start so that routing can
 * converge */
#define START_DELAY 60

typedef union network_data_t {
	uint8_t current_temp;
	uint8_t current_volt;
} network_data_t;

#endif