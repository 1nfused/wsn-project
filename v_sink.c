#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ip/uip-debug.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include <string.h>

#define DEBUG DEBUG_PRINT

#include "v_sink.h"
#include "common.h"

static struct uip_udp_conn *sink_conn;
static uint16_t count;

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

/*---------------------------------------------------------------------------*/
PROCESS(mcast_sink_process, "Multicast Sink");
AUTOSTART_PROCESSES(&mcast_sink_process);
/*---------------------------------------------------------------------------*/
static void tcpip_handler(void) {
  if(uip_newdata()) {
    printf("%s\n",((char *)uip_appdata));
    printf("Printing IPv6 address.\n");

    char src_address[100]; // Set len of ipv6 in .h
    memset(src_address, 0, sizeof(UIP_IP_BUF->srcipaddr.u8));
    memcpy(src_address, (void *)&UIP_IP_BUF->srcipaddr.u8, sizeof(UIP_IP_BUF->srcipaddr.u16));
    printf("Nothing to see here?\n");
    printf("Address: %s\n", sizeof(src_address));
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

/* ---------------- MAIN THREAD ---------------- */
PROCESS_THREAD(mcast_sink_process, ev, data) {
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);
  printf("Multicast Engine: '%s'\n", UIP_MCAST6.name);

  if(join_mcast_group() == NULL) {
    PRINTF("Failed to join multicast group\n");
    PROCESS_EXIT();
  }

  count = 0;

  sink_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(sink_conn, UIP_HTONS(MCAST_SINK_UDP_PORT));

  printf("Listening: ");
  PRINT6ADDR(&sink_conn->ripaddr);
  printf(" local/remote port %u/%u\n",
        UIP_HTONS(sink_conn->lport), UIP_HTONS(sink_conn->rport));

  printf("Waiting for tcp event.\n");
  while(1) {
    PROCESS_WAIT_EVENT();
    leds_off(LEDS_ALL);
    if(ev == tcpip_event) {
      printf("\nGot event!\n");
      tcpip_handler();
    } else if((ev == sensors_event) && (data == &button_sensor)){
      printf("\nButton event!!\n");
    }

    leds_on(LEDS_ALL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
