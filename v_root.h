#ifndef V_ROOT_H
#define V_ROOT_H

#define MAX_PAYLOAD_LEN 120

#define SEND_INTERVAL CLOCK_SECOND /* clock ticks */
#define ITERATIONS 100 /* messages */
#define DEBUG DEBUG_PRINT
#define SEND_INTERVAL CLOCK_SECOND /* clock ticks */
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

/* Start sending messages START_DELAY secs after we start so that routing can
 * converge */
#define START_DELAY 60

// Unions for network data. Only one field can be populated!
typedef union network_data_min_t {
	uint8_t temp;
	uint8_t volt;
	uint8_t vib;
} network_data_min_t;

typedef union network_data_max_t {
	uint8_t temp;
	uint8_t volt;
	uint8_t vib;
} network_data_max_t;

typedef union network_data_avg_t {
	uint8_t temp;
	uint8_t volt;
	uint8_t vib;
} network_data_avg_t;

// Methods protos definitions


#endif