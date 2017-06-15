#ifndef V_SINK_H
#define V_SINK_H

#define IPV6_BASE_SIZE				32
#define DEBUG DEBUG_PRINT
#define UDP_CLIENT_PORT 			8765
#define UDP_SERVER_PORT 			5678

#define DEFAULT_TEMP_THRESHOLD		10
#define DEFAULT_VIB_THRESHOLD		1000

static void send_data();
void get_temperature(uint16_t *temperature);
void get_normed_vibr(uint16_t *vibration);
void check_alarms(bool force_report);

#endif