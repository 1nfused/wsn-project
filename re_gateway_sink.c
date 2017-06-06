// Contiki includes mixed with C standard libs which
// prevents circular dependency includes 
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "apps/shell/shell.h"
#include "dev/uart0.h"
#include "contiki.h"
#include "dev/leds.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// App includes
#include "re_gateway_sink.h"
#include "common.h"


PROCESS(re_gateway_sink, "Starting real gateway sink");
AUTOSTART_PROCESSES(&re_gateway_sink);

PROCESS_THREAD(re_gateway_sink, ev, data){
	PROCESS_BEGIN();

	// Init uart communication
	uart0_set_input(serial_line_input_byte);
	serial_line_init();
	
	while(1) {
		// Turn off all LEDS
		leds_off(LEDS_ALL);
		printf("Getting data\n");

		char buff[MAX_CMD_LENGTH];
		PROCESS_WAIT_EVENT();
		if((ev == serial_line_event_message) && data != NULL) {
			leds_on(LEDS_ALL);
			strncpy(buff, data, MAX_CMD_LENGTH);
			printf("Buffer: %s\n", buff);

		}
	}

	PROCESS_END();
}
