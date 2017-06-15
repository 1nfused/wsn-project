#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ip/uip-debug.h"
#include "net/rpl/rpl.h"
#include "net/rime/rime.h"

#include <string.h>
#include <stdlib.h>

#include "v_root.h"
#include "common.h"

static struct uip_udp_conn * mcast_conn;
static struct uip_udp_conn *server_conn;
static char message[MAX_PAYLOAD_LEN];
static uint32_t seq_id;

// TODO:
// Parse method for alarms & normal data.
// Number of packets sent, so we know how many to except & a delay
// in case not all of 'em arrive.

// Global state
static char *current_command;
static int active = 0;

// Network statisical data
static network_data_min_t *network_min;
static network_data_max_t *network_max;
static network_data_avg_t *network_avg;

PROCESS(rpl_root_process, "Virtual Root Mote");
AUTOSTART_PROCESSES(&rpl_root_process);

// Print usage
static void usage(void) {
    printf("List of avaliable commands:\n");
    printf("Z:MULT - Start multicast process\n");
    printf("Z:H? - Heart beat function. Check what motes are avaliable\n");
    printf("Z:S:T:MIN? - Get min temperature\n");
    printf("Z:S:T:MAX? - Get max temperature\n");
    printf("Z:S:T:AVG? - Get avg temperature\n");
    printf("Z:S:T:MIN? - Get min voltage\n");
    printf("Z:S:T:MAX? - Get max voltage\n");
    printf("Z:S:T:AVG? - Get avg voltage\n");
    printf("Z:S:T:MIN? - Get min vibrations\n");
    printf("Z:S:T:MAX? - Get max vibrations\n");
    printf("Z:S:T:AVG? - Get avg vibrations\n");
}

void parseResult(char **appdata, uint16_t *ptr, int type){
    char *pChr = strtok (*appdata, "-:");
    switch(type) {
        case 1:
            while (pChr != NULL) {
                printf ("%s\n", pChr);
                pChr = strtok(NULL, "-:");
            }
            break;
        case 2:
            break;
        default:
            printf("Invalid data.\n");
    }
    *ptr = 10;
}

// Handle unicast receive
static void tcpip_handler(void) {
    char *appdata;
    if(uip_newdata()) {
        appdata = (char *)uip_appdata;
        appdata[uip_datalen()] = 0;
        
        if(strcmp(current_command, GET_HEART_BEAT) == 0) {
            active++;
            printf("Currently active: %d\n", active);
        } else if(strcmp(current_command, GET_TEMP_MIN) == 0){
            
            // Parse recevied result
            uint16_t temperature;
            parseResult(&appdata, &temperature, 1);
            if (network_min->temp > temperature) {
                network_min->temp = temperature;
            }

        } else if(strcmp(current_command, GET_TEMP_MAX) == 0){
            network_max->temp = 2;
        } else if(strcmp(current_command, GET_TEMP_AVG) == 0){
            //network_min.min_temp = atof(appdata);
        } else if(strcmp(current_command, GET_VIB_MIN) == 0){
            network_min->vib = 5;
        } else if(strcmp(current_command, GET_VIB_MAX) == 0){
            network_max->vib = 1;
        } else if(strcmp(current_command, GET_VIB_AVG) == 0){
            printf("Got new avg vibrations: %s\n", appdata);
        // No command was executed
        } else {
            printf("Alarams: %s\n", appdata);
        }
    }
}

static void multicast_send(char *command) {
    printf("Sending: %s\n", command);
    seq_id++;
    uip_udp_packet_send(mcast_conn, command, strlen(&command[0]));
}

static void prepare_mcast(void) {
    uip_ipaddr_t ipaddr;
    uip_ip6addr(&ipaddr, 0xFF1E,0,0,0,0,0,0x89,0xABCD);
    mcast_conn = udp_new(&ipaddr, UIP_HTONS(MCAST_SINK_UDP_PORT), NULL);
}

static void set_own_addresses(void) {
    int i;
    uint8_t state;
    rpl_dag_t *dag;
    uip_ipaddr_t ipaddr;

    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

    PRINTF("Our IPv6 addresses:\n");
    for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
        state = uip_ds6_if.addr_list[i].state;
        if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state
            == ADDR_PREFERRED)) {
            PRINTF("  ");
            PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
            PRINTF("\n");
            if(state == ADDR_TENTATIVE) {
                uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
            }
        }
    }

    /* Become root of a new DODAG with ID our global v6 address */
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
    if(dag != NULL) {
        rpl_set_prefix(dag, &ipaddr, 64);
        PRINTF("Created a new RPL dag with ID: ");
        PRINT6ADDR(&dag->dag_id);
        PRINTF("\n");
    }
}

/* MAIN THREAD */
PROCESS_THREAD(rpl_root_process, ev, data) {
    // Open unicast connection
    static struct etimer et;
    static struct etimer et_sensor;
    PROCESS_BEGIN();
    PRINTF("Multicast Engine: '%s'\n", UIP_MCAST6.name);
    NETSTACK_MAC.off(1);
    set_own_addresses();
    prepare_mcast();

    uart0_set_input(serial_line_input_byte);
    serial_line_init();

    accm_init();
    tmp102_init();

    server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
    if(server_conn == NULL) {
        PRINTF("*ROOT: No UDP connection available, exiting the process!\n");
        PROCESS_EXIT();
    }
    udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));
   
    etimer_set(&et_sensor, CLOCK_SECOND);
    while(1) {
        PROCESS_YIELD();

        if(etimer_expired(&et)){

            PRINTF("Timer expired.\n");
            leds_off(LEDS_GREEN);

            // Check for temperature
            etimer_set(&et_sensor, DELAY_CHECK_SENSORS_VALUE * CLOCK_SECOND);
        }

        /* Wait for mote responses */
        if(ev == tcpip_event) {
            printf("Got new event!\n");
            tcpip_handler();
        }

        // Parse serial event and send multicast data
        if(ev == serial_line_event_message && data != NULL){
            current_command = (char *)data;
            //printf("Currently executing: %s\n", &current_command[0]);
            // Start multicast
            if(strcmp(GET_USAGE, current_command) == 0) {
                usage();
            } else {
                active = 0;
                multicast_send(current_command);
            }

        }
  }
  PROCESS_END();
}
