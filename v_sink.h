#ifndef V_SINK_H
#define V_SINK_H

#define IPV6_BASE_SIZE				32
#define DEBUG DEBUG_PRINT
#define UDP_CLIENT_PORT 			8765
#define UDP_SERVER_PORT 			5678
#define THRESH_HOLD_TEMPERATURE 	7
#define THRESH_HOLD_VIBRATION 		2

static void send_data();
void get_temperature(uint16_t *temperature);
void get_normed_vibr(uint16_t *vibration);

#endif