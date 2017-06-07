#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ip/uip-debug.h"
#include "net/rpl/rpl.h"
#include "net/rime/rime.h"

#include <string.h>

#include "v_root.h"
#include "common.h"

static struct uip_udp_conn * mcast_conn;
static struct uip_udp_conn *server_conn;
static char message[MAX_PAYLOAD_LEN];
static uint32_t seq_id;
static network_data_t *network;
static long temperature;
static long vibration;

PROCESS(rpl_root_process, "Virtual Root Mote");
AUTOSTART_PROCESSES(&rpl_root_process);

// Handle unicast receive
static void tcpip_handler(void) {
    char *appdata;
    printf("In function.\n");
    if(uip_newdata()) {
        appdata = (char *)uip_appdata;
        appdata[uip_datalen()] = 0;
        printf("%s\n", appdata);
    }
}


static void multicast_send(char *command) {
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

    PRINTF("*ROOT: Creating new udp connection\n"); 
    server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
    if(server_conn == NULL) {
        PRINTF("*ROOT: No UDP connection available, exiting the process!\n");
        PROCESS_EXIT();
    }
    udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));

    PRINTF("*ROOT: Created a server connection with remote address ");
    PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport), UIP_HTONS(server_conn->rport));
    
    etimer_set(&et_sensor, CLOCK_SECOND);
    while(1) {
        PROCESS_YIELD();

        if(etimer_expired(&et)){
            //temperature = get_temperature();
            //vibration = get_normed_vibr();

            PRINTF("Timer expired.\n");
            leds_off(LEDS_GREEN);

            // Check for temperature
            etimer_set(&et_sensor, DELAY_CHECK_SENSORS_VALUE * CLOCK_SECOND);
        }

        /* Wait for mote responses */
        if(ev == tcpip_event) {
            printf("Got tcpip event.\n");
            tcpip_handler();
        }

        // Parse serial event and send multicast data
        if(ev == serial_line_event_message && data != NULL){
            leds_on(LEDS_GREEN);
            multicast_send(data);
        }
  }
  PROCESS_END();
}
