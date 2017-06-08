#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ip/uip-debug.h"
#include "net/rpl/rpl.h"
#include "node-id.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#include "v_sink.h"
#include "common.h"

static struct uip_udp_conn *client_conn;
static struct uip_udp_conn *sink_conn;
static uint16_t count;
static uip_ip6addr_t src_addr;
static char buffer[MAX_PAY_LOAD];
static float THRESH_HOLD_TEMPERATURE = 10;
static float THRESH_HOLD_VIBRATION = 2;

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

/*---------------------------------------------------------------------------*/
PROCESS(mcast_sink_process, "Multicast Sink");
AUTOSTART_PROCESSES(&mcast_sink_process);

static long get_temperature(){
    // Example from: contiki/examples/z1/tst-tmp102.c
    int16_t raw;
    uint16_t absraw;    
    long temperature = 0;
    
    raw = tmp102_read_temp_raw();
    absraw = raw;   
    printf("RAW: %d\n", raw);
    temperature = (absraw >> 8);  // temp_int
    temperature *= 10000;
    temperature += ((absraw >> 4) % 16) * 625;  // Info in 1/10000 of degree  // temp_frac  
    temperature = temperature / 1000;
    printf("Acq temp: %d\n", (int)temperature);
    return temperature;
    //return 100 + node_id;
}

int abs(int v) {
    return v * ((v>0) - (v<0));
}

int get_normed_vibr() {
    int x, y, z, norm;
    x = accm_read_axis(X_AXIS);
    y = accm_read_axis(Y_AXIS);
    z = accm_read_axis(Z_AXIS);
    norm = abs(x) + abs(y) + abs(z);
    norm = norm % 1000;
    return norm;
}

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
        } else if((strcmp(appdata, GET_TEMP_MIN) == 0) ||
                  (strcmp(appdata, GET_TEMP_MAX) == 0) ||
                  (strcmp(appdata, GET_TEMP_AVG) == 0)){
            double t = (double)get_temperature();
            printf("TEmp: %f\n", t);
            sprintf(buffer, NODE_RES_FORMAT, node_id, t);
        } else if((strcmp(appdata, GET_VIB_MIN) == 0) ||
                  (strcmp(appdata, GET_VIB_MAX) == 0) ||
                  (strcmp(appdata, GET_VIB_AVG) == 0)) {
            sprintf(buffer, NODE_RES_FORMAT, node_id, (double)get_normed_vibr());
        } else if((strcmp(appdata, GET_VOLT_MIN) == 0) ||
                  (strcmp(appdata, GET_VOLT_MAX) == 0) ||
                  (strcmp(appdata, GET_VOLT_AVG) ==0)) {
            printf("NOT IMPLEMENTED\n");
        }

        printf("%s\n", &buffer[0]);
        uip_ipaddr_copy(&src_addr, &UIP_IP_BUF->srcipaddr);
        // Send data back to the client
        send_data();
    }
    return;
}

static void send_data(void) {
    uip_udp_packet_sendto(client_conn, buffer, strlen(buffer), &src_addr, UIP_HTONS(UDP_SERVER_PORT));
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

    etimer_set(&et_sensor, CLOCK_SECOND);

    while(1) {
        PROCESS_WAIT_EVENT();
        // Alarms
        if(etimer_expired(&et_sensor)) {
            int temperature = (int)get_temperature();
            int vibration = (int)get_normed_vibr();

            // Check temp thresh
            if(temperature > THRESH_HOLD_TEMPERATURE){
                sprintf(buffer, NODE_RES_FORMAT, node_id, (double)temperature);
                send_data();
            } 
            // Check vibration thresh
            if(vibration > THRESH_HOLD_VIBRATION) {
                sprintf(buffer, NODE_RES_FORMAT, node_id, (double)vibration);
                send_data();
            }
            // Set sensor check delay
            etimer_set(&et_sensor, DELAY_CHECK_SENSORS_VALUE * CLOCK_SECOND);
        }

        if(ev == tcpip_event) {
            printf("Got event!\n");
            tcpip_handler();
        }

        rpl_instance_t *instance = rpl_get_instance(RPL_DEFAULT_INSTANCE);
        rpl_dag_t *any_dag = rpl_get_any_dag();
        if(instance != NULL && any_dag != NULL){
            uip_ipaddr_copy(&src_addr, &any_dag->dag_id);
            leds_on(LEDS_RED);
        } else {
            leds_off(LEDS_RED);
        }
    }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
