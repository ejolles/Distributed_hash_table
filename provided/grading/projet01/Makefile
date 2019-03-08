CFLAGS += -std=c11
CFLAGS += -g
CFLAGS += -Wall -pedantic -O2
LDLIBS += -lcheck -lm -lrt -pthread -lsubunit

# ==================================== #
all: test-hashtable    \
	 week04            \
	 pps-launch-server \
	 pps-client-put    \
	 pps-client-get    \
	 log-packets
# ------------------------------------ #

# WEEK 04
curstom_util.o: config.h error.h
hashatable.o: custom_util.h error.h util.h
test-hashtable: hashtable.o custom_util.o
week04: hashtable.o custom_util.o

# WEEK 05 & WEEK 06
system.o: error.h
client.o: args.h config.h error.h node.h node_list.h ring.h
network.o: custom_util.h error.h
node.o: error.h util.h system.h
node_list.o: custom_util.h config.h error.h node.h

pps-launch-server: system.o hashtable.o custom_util.o
pps-client-put: client.o custom_util.o network.o system.o node.o node_list.o system.o
pps-client-get: client.o custom_util.o network.o system.o node.o node_list.o system.o
log-packets: Makefile.log-packets
	make -f $^

#correcteur: il pourrait avoir une règle 'clean' pour supprimer les .o et les executables (-0.25)