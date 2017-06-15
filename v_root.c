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
static uint32_t seq_id;

// Global state
static char *current_command;
static int active_motes = 0;
int data_recv_vib = 0;
int data_recv_temp = 0;

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

void parseResult(char **appdata, uint16_t *ptr){
    char *pChr = strtok (*appdata, ":");
    pChr = strtok(NULL, ":");
    *ptr = atoi(pChr);
    printf("POINTER DATA: %d\n", *ptr);
}

// Handle unicast receive
static void tcpip_handler(void) {
    char *appdata;
    uint16_t temperature;
    uint16_t vibrations;

    if(uip_newdata()) {
        appdata = (char *)uip_appdata;
        appdata[uip_datalen()] = 0;
        
        // Network hearthbeat functionality
        if(strcmp(current_command, GET_HEART_BEAT) == 0) {
            active_motes++;
            printf("Currently active: %d\n", active_motes);
        } else if(strcmp(current_command, GET_TEMP_MIN) == 0){
            // Parse received result
            parseResult(&appdata, &temperature);
            if (network_min->temp < temperature) {
                network_min->temp = temperature;
            }
            printf("Min temp so far: %d\n", network_min->temp);
        } else if(strcmp(current_command, GET_TEMP_MAX) == 0){
            parseResult(&appdata, &temperature);
            if (network_max->temp < temperature) {
                network_max->temp = temperature;
            }
            printf("Max temp so far: %d\n", network_max->temp);
        } else if(strcmp(current_command, GET_TEMP_AVG) == 0){
            parseResult(&appdata, &temperature);
            network_avg->temp += temperature;
            data_recv_temp += 1;
            printf("Average current temperature: %d\n", 
                    network_avg->temp / data_recv_temp);
            if(data_recv_temp > active_motes) {
                data_recv_temp = 0;
            }
        } else if(strcmp(current_command, GET_VIB_MIN) == 0){
            parseResult(&appdata, &vibrations);
            if (network_min->vib > vibrations) {
                network_min->vib = vibrations;
            }
            printf("Min vibrations so far: %d\n", network_min->vib);
        } else if(strcmp(current_command, GET_VIB_MAX) == 0){
            parseResult(&appdata, &vibrations);
            if (network_max->vib < vibrations) {
                network_max->vib = vibrations;
            }
            printf("Max vibrations so far: %d\n", network_max->vib);
        } else if(strcmp(current_command, GET_VIB_AVG) == 0){
            parseResult(&appdata, &vibrations);
            network_avg->vib += temperature;
            data_recv_temp += 1;
            printf("Average current temperature: %d\n", 
                    network_avg->vib / data_recv_vib);
            if(data_recv_vib > active_motes) {
                data_recv_vib = 0;
            }
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
    printf("Multicast Engine: '%s'\n", UIP_MCAST6.name);
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
                active_motes = 0;
                multicast_send(current_command);
            }

        }
  }
  PROCESS_END();
}
