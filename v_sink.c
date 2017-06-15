#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ip/uip-debug.h"
#include "net/rpl/rpl.h"
#include "node-id.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "v_sink.h"
#include "common.h"

static struct uip_udp_conn *client_conn;
static struct uip_udp_conn *sink_conn;
static uint16_t count;
static uip_ip6addr_t src_addr;
static char buffer[MAX_PAY_LOAD];
uint16_t temperature;
uint16_t vibration;

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

uint16_t threshold_temperature;
uint16_t threshold_vibration;

/*---------------------------------------------------------------------------*/
PROCESS(mcast_sink_process, "Multicast Sink");
AUTOSTART_PROCESSES(&mcast_sink_process);

void get_temperature(uint16_t *temperature){
    // Example from: contiki/examples/z1/tst-tmp102.c
    int16_t raw;
    uint16_t absraw;    
    long temp = 0;
    
    raw = tmp102_read_temp_raw();
    absraw = raw;   
    temp = (absraw >> 8);  // temp_int
    temp *= 10000;
    temp += ((absraw >> 4) % 16) * 625;  // Info in 1/10000 of degree  // temp_frac  
    temp = temp / 1000;
    *temperature = 8.12;
}

int abs(int v) {
    return v * ((v>0) - (v<0));
}

void get_normed_vibr(uint16_t *vibration) {
    int x, y, z, norm;
    x = accm_read_axis(X_AXIS);
    y = accm_read_axis(Y_AXIS);
    z = accm_read_axis(Z_AXIS);
    norm = abs(x + y + z);
    norm = norm % 1000;
    *vibration = norm;
}

/*---------------------------------------------------------------------------*/
static void tcpip_handler(void) {
    char *appdata;
    
    if(uip_newdata()) {
        count++;
        appdata = (char *)uip_appdata;
        appdata[uip_datalen()] = 0;
        char *pChr = strtok (appdata, "-");
        uint16_t value = atoi(strtok(NULL, "-"));   
        // Parse command
        if (strcmp(appdata, GET_HEART_BEAT) == 0){
        
            memcpy(buffer, "1", strlen("1"));
        
        } else if((strcmp(appdata, GET_TEMP_MIN) == 0) ||
                  (strcmp(appdata, GET_TEMP_MAX) == 0) ||
                  (strcmp(appdata, GET_TEMP_AVG) == 0)){

            sprintf(buffer, "M:1-D:8.12\n"); 
        
        } else if((strcmp(appdata, GET_VIB_MIN) == 0) ||
                  (strcmp(appdata, GET_VIB_MAX) == 0) ||
                  (strcmp(appdata, GET_VIB_AVG) == 0)) {
            get_normed_vibr(&vibration);
            sprintf(buffer, NODE_RES_FORMAT, node_id, vibration);

        } else if(strcmp(pChr, SET_TEMP_THR) == 0) {
            printf("%d\n", value);
            threshold_temperature = value;
            printf("new tempeature threshold: %d\n", threshold_temperature);
        
        } else if(strcmp(pChr, SET_VIB_THR) == 0) {
            threshold_vibration = value;
            printf("NEW VIB THRESHOLD: %d\n", threshold_vibration);
        }else {
            printf("Invalid command!\n");   
        }
        
        uip_ipaddr_copy(&src_addr, &UIP_IP_BUF->srcipaddr);
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
        //printf("Joined multicast group ");
        PRINT6ADDR(&uip_ds6_maddr_lookup(&addr)->ipaddr);
        //printf("\n");
    }
    return rv;
}

void check_alarms(bool force_report) {
    leds_on(LEDS_RED);
    bool alarmed_state = false;
    /* Alarms core compilement problem. */
    get_temperature(&temperature);
    get_normed_vibr(&vibration);

    if(temperature > threshold_temperature){
        sprintf(buffer, NODE_ALRM_T_FORMAT, node_id, temperature);
        alarmed_state = true;
    } else if(vibration > threshold_vibration) {
        sprintf(buffer, NODE_ALRM_V_FORMAT, node_id, vibration);
        alarmed_state = true;
    } else {
        alarmed_state = false;
    }
    // Turn off alarmed state if no threshold is 
    if(alarmed_state) {
        send_data();
    }

    // Alarms
    if(force_report == true){
        printf("Triggered!\n");
        sprintf(buffer, NODE_ALRM_T_FORMAT, node_id, 2);
        send_data();
    }
}

/* MAIN THREAD */
PROCESS_THREAD(mcast_sink_process, ev, data) {
    threshold_temperature = DEFAULT_TEMP_THRESHOLD;
    threshold_vibration = DEFAULT_VIB_THRESHOLD;

    static struct etimer et_sensor;
    PROCESS_BEGIN();
    if(join_mcast_group() == NULL) {
        PROCESS_EXIT();
    }

    sink_conn = udp_new(NULL, UIP_HTONS(0), NULL);
    udp_bind(sink_conn, UIP_HTONS(MCAST_SINK_UDP_PORT));

    // Activate sensors
    SENSORS_ACTIVATE(button_sensor);
    accm_init();
    tmp102_init();

    count = 0;
    // Create & bind sink with root
    client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL); 
    if(client_conn == NULL) {
        PROCESS_EXIT();
    }

    udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));
    etimer_set(&et_sensor, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));
    while(1) {
        PROCESS_WAIT_EVENT();
        if(etimer_expired(&et_sensor)) {
            check_alarms(false);
            etimer_set(&et_sensor, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));
        }

        if(ev == tcpip_event) {
            tcpip_handler();
        }

        // Force mote report directly to root
        if((ev == sensors_event) && (data == &button_sensor)) {
            printf("Sending force alarm.\n");
            check_alarms(true);
        }

        // Create DAG
        rpl_instance_t *instance = rpl_get_instance(RPL_DEFAULT_INSTANCE);
        rpl_dag_t *any_dag = rpl_get_any_dag();
        if(instance != NULL && any_dag != NULL){
            uip_ipaddr_copy(&src_addr, &any_dag->dag_id);
            leds_on(LEDS_GREEN);
        } else {
            leds_off(LEDS_GREEN);
        }
    }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
