#include "stdio.h"
#include "contiki.h"
#include "dev/serial-line.h"
#include "apps/shell/shell.h"
#include "dev/button-sensor.h"
#include "dev/uart0.h"
#include "random.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "net/linkaddr.h"

#include "net/netstack.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/uart0.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"


#define VIRTUAL_MOTE 	0
#define SLEEP			1e9
int j = 0;

PROCESS(v_gateway_sink, "Virtual gateway sink.");
AUTOSTART_PROCESSES(&v_gateway_sink);

PROCESS_THREAD(v_gateway_sink, ev, data){
	//static struct etimer et;
	PROCESS_BEGIN();

	while(1) {
		/* Delay 2-4 seconds */
		//etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));
		//for (j = 0; j < 100000; ++j){ }
		PRINTF("I am sending you through the gates of hell.\n");
		printf("I am sending you through the gates of hell\n");
		break;
	}
	
	PROCESS_END();
}
