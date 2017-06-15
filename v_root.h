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
	uint16_t temp;
	uint16_t volt;
	uint16_t vib;
} network_data_min_t;

typedef union network_data_max_t {
	uint16_t temp;
	uint16_t volt;
	uint16_t vib;
} network_data_max_t;

typedef union network_data_avg_t {
	uint16_t temp;
	uint16_t volt;
	uint16_t vib;
} network_data_avg_t;

// Methods protos definitions


#endif