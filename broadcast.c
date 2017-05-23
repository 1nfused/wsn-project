#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"

#include <stdio.h>

PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);

/* BROADCAST */
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = { broadcast_recv };
static struct broadcast_conn broadcast;

PROCESS_THREAD(example_broadcast_process, ev, data) {
    static struct etimer et;

    PROCESS_EXITHANDLER(broadcast_close(&broadcast));
    PROCESS_BEGIN();

    // Open broadcast listener at 100
    broadcast_open(&broadcast, 100, &broadcast_call);
    unicast_open(&uc, 110, &unicast_callbacks);

    while(1) {
        linkaddr_t addr;
        /* Delay 2-4 seconds */
        etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        
        /* Create a simple answer to the server */
        char message[200];
        strcpy(message, "I thought this was America...");

        /* Send broadcast message */
        printf("Sending broadcast message ...\n");
        packetbuf_copyfrom(&message[0], strlen(message));
        broadcast_send(&broadcast);
    }
    PROCESS_END();
}