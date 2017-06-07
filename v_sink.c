#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ip/uip-debug.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#include "v_sink.h"
#include "common.h"

static struct uip_udp_conn *client_conn;
static struct uip_udp_conn *sink_conn;
static uint16_t count;
static uip_ip6addr_t src_addr = NULL;
static char buffer[MAX_PAY_LOAD];

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

/*---------------------------------------------------------------------------*/
PROCESS(mcast_sink_process, "Multicast Sink");
AUTOSTART_PROCESSES(&mcast_sink_process);

static int seq_id = 0;
static int reply = 0;

/*---------------------------------------------------------------------------*/
static void tcpip_handler(void) {
    char *appdata;
    
    if(uip_newdata()) {
        count++;
        appdata = (char *)uip_appdata;
        appdata[uip_datalen()] = 0;
        printf("Sink received: %s\n", &appdata[0]);
        // Parse command
        if (strcmp(appdata, GET_HEART_BEAT) == 0){
            memcpy(buffer, "1", strlen("1"));
        }

        // Copy source ip address
        if(src_addr != NULL) {
            uip_ipaddr_copy(&src_addr, &UIP_IP_BUF->srcipaddr);
        }
        
        PRINTF("Address: %s\n", appdata);
        uip_udp_packet_sendto(
            client_conn, 
            buffer,
            strlen(buffer), 
            &src_addr, 
            UIP_HTONS(UDP_SERVER_PORT));
    }
    return;
}

static uip_ds6_maddr_t *join_mcast_group(void){
    uip_ipaddr_t addr;
    uip_ds6_maddr_t *rv;

    /* First, set our v6 global */
    uip_ip6addr(&addr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&addr, &uip_lladdr);
    uip_ds6_addr_add(&addr, 0, ADDR_AUTOCONF);

    /*
    * IPHC will use stateless multicast compression for this destination
    * (M=1, DAC=0), with 32 inline bits (1E 89 AB CD)
    */
    uip_ip6addr(&addr, 0xFF1E,0,0,0,0,0,0x89,0xABCD);
    rv = uip_ds6_maddr_add(&addr);

    if(rv) {
        printf("Joined multicast group ");
        PRINT6ADDR(&uip_ds6_maddr_lookup(&addr)->ipaddr);
        printf("\n");
    }
    return rv;
}


/* MAIN THREAD */
PROCESS_THREAD(mcast_sink_process, ev, data) {
    // Variables definition
    static struct etimer et;
    static struct etimer et_sensor;
    
    PROCESS_BEGIN();
    PRINTF("STARTING SINK!\n");

    if(join_mcast_group() == NULL) {
        PRINTF("Failed to join multicast group\n");
        PROCESS_EXIT();
    }

    sink_conn = udp_new(NULL, UIP_HTONS(0), NULL);
    udp_bind(sink_conn, UIP_HTONS(MCAST_SINK_UDP_PORT));

    // Activate sensors
    SENSORS_ACTIVATE(button_sensor);
    accm_init();
    tmp102_init();

    // Initialize UART
    uart0_set_input(serial_line_input_byte);
    serial_line_init();

    count = 0;

    // Create & bind sink with root
    client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL); 
    if(client_conn == NULL) {
        PRINTF("No UDP connection available, exiting the process!\n");
        PROCESS_EXIT();
    }
    udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));

    PRINTF("*SINK: Created a connection with the server ");
    PRINTF(" local/remote port %u/%u\n", UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

    while(1) {
        printf("Waiting for event!\n");
        PROCESS_WAIT_EVENT();
        printf("Gotcha!\n");
        if(ev == tcpip_event) {
            printf("Got event!\n");
            tcpip_handler();
        }

        rpl_instance_t *instance = rpl_get_instance(RPL_DEFAULT_INSTANCE);
        rpl_dag_t *any_dag = rpl_get_any_dag();
        if(instance != NULL && any_dag != NULL){
            uip_ipaddr_copy(&server_ipaddr, &any_dag->dag_id);
            leds_on(LEDS_RED);
        } else {
            leds_off(LEDS_RED);
        }
    }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
