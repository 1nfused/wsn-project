all: rime_server sensor_server
server: rime_server
sensor: sensor_server

DEFINES += PROJECT_CONF_H=\"project-conf.h\"

CONTIKI_WITH_RIME = 1
CONTIKI = /home/user/contiki
include $(CONTIKI)/Makefile.include

