#ifndef V_ROOT_H
#define V_ROOT_H

#define MAX_PAYLOAD_LEN 120

#define SEND_INTERVAL CLOCK_SECOND /* clock ticks */
#define ITERATIONS 100 /* messages */

/* Start sending messages START_DELAY secs after we start so that routing can
 * converge */
#define START_DELAY 60

// Contiki function
static void multicast_send(void);
static void prepare_mcast(void);
static void set_own_addresses(void);

#endif