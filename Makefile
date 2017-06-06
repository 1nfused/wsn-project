all: vroot vsink vgatewaysink regatewaysink
vroot: v_root
vsink: v_sink
vgatewaysink: v_gateway_sink
regatewaysink: re_gateway_sink

CONTIKI_WITH_IPV6 = 1
CONTIKI_WITH_RIME = 1

DEFINES += PROJECT_CONF_H=\"project-conf.h\"
MODULES += core/net/ipv6/multicast

CONTIKI = /home/user/contiki
include $(CONTIKI)/Makefile.include

