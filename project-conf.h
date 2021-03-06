#define BROADCAST_CHANNEL	100
#define UNICAST_CHANNEL		110
#define MESH_CHANNEL		120

#define LED_ON_INTERVAL		CLOCK_SECOND

#include "net/ipv6/multicast/uip-mcast6-engines.h"

/* Change this to switch engines. Engine codes in uip-mcast6-engines.h */
#define UIP_MCAST6_CONF_ENGINE UIP_MCAST6_ENGINE_ROLL_TM

/* For Imin: Use 16 over NullRDC, 64 over Contiki MAC */
#define ROLL_TM_CONF_IMIN_1         64

#undef UIP_CONF_IPV6_RPL
#undef UIP_CONF_ND6_SEND_RA
#undef UIP_CONF_ROUTER
#define UIP_CONF_ND6_SEND_RA         0
#define UIP_CONF_ROUTER              1
#define UIP_MCAST6_ROUTE_CONF_ROUTES 1

#undef UIP_CONF_TCP
#define UIP_CONF_TCP 0

/* Code/RAM footprint savings so that things will fit on our device */
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#undef UIP_CONF_MAX_ROUTES
#define NBR_TABLE_CONF_MAX_NEIGHBORS  10
#define UIP_CONF_MAX_ROUTES           10

