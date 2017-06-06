#include "stdio.h"
#include "contiki.h"
#include "random.h"
#include "dev/serial-line.h"
#include "apps/shell/shell.h"
#include "dev/uart0.h"

// App includes
#include "v_gateway_sink.h"
#include "common.h"

static struct etimer et;

PROCESS(v_gateway_sink, "Virtual gateway sink.");
AUTOSTART_PROCESSES(&v_gateway_sink);

PROCESS_THREAD(v_gateway_sink, ev, data){
	//static struct etimer et;
	PROCESS_BEGIN();

	// Init uart communication
	uart0_set_input(serial_line_input_byte);
	serial_line_init();

	while(1) {
		// Wait for one second
		etimer_set(&et, CLOCK_SECOND*5);
		PROCESS_WAIT_EVENT();
		printf("get_max_temp()\n");
	}
	
	etimer_reset(&et);	
	PROCESS_END();
}
