#include <stdio.h>

#include "stdio.h"
#include "dev/serial-line.h"
#include "apps/shell/shell.h"
#include "dev/button-sensor.h"

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "net/linkaddr.h"

#include "net/netstack.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#if CONTIKI_TARGET_Z1
#include "dev/uart0.h"
#else
#include "dev/uart1.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"


#define VIRTUAL_MOTE 0
int i = 1;

PROCESS(re_gateway_sink, "Real gateway sink");
AUTOSTART_PROCESSES(&re_gateway_sink);

PROCESS_THREAD(re_gateway_sink, ev, data){
	PROCESS_BEGIN();

	// Init uart communication
	uart0_set_input(serial_line_input_byte);
	serial_line_init();
	while(1) {
		PROCESS_YIELD();
		if((ev == serial_line_event_message) && (data != NULL)) {
			PRINTF("This shit is now real!!\n");
			i++;
		}
	}
	
	PROCESS_END();
}
