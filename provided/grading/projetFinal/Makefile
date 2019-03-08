CFLAGS += -std=c11
CFLAGS += -pedantic -O2
CFLAGS += -W -Wall -Wextra -Wcast-align -Wfloat-equal -Wpointer-arith -Wformat=2 -Winit-self -Wuninitialized -Wshadow -Wno-unused-parameter -Wbad-function-cast -Wunreachable-code -Wwrite-strings -Wconversion
#CFLAGS += -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wcast-qual
LDLIBS += -lcheck -lm -lrt -pthread -lsubunit -lcrypto

EXEC = test-hashtable    \
	   week04            \
	   pps-launch-server \
	   pps-client-put    \
	   pps-client-get    \
	   log-packets.so    \
	   pps-list-nodes    \
	   pps-dump-node     \
	   pps-client-cat    \
	   pps-client-substr \
	   pps-client-find   \
	   test-servers      \
	   test-parser


# ==================================== #
all: $(EXEC)
# ------------------------------------ #

# Compiling non-executables
args.o: args.c args.h error.h
client.o: client.c client.h args.h config.h error.h node.h node_list.h ring.h util.h
config.o: config.h
error.o: error.h error.c
hashtable.o: hashtable.c hashtable.h error.h util.h
network.o: network.c network.h client.h error.h hashtable.h node.h util.h
node.o: node.c node.h error.h util.h system.h
node_list.o: node_list.c node_list.h config.h error.h node.h
ring.o: ring.c ring.h error.h hashtable.h node_list.h
system.o: system.c system.h error.h system.h
test.o: test.h error.h
util.o: util.c util.h

# Compiling executables
pps-client-cat.o: pps-client-cat.c args.h client.h error.h network.h node.h system.h util.h
pps-client-find.o: pps-client-find.c client.h error.h network.h node.h system.h
pps-client-get.o: pps-client-get.c client.h error.h network.h node.h system.h
pps-client-put.o: pps-client-put.c client.h error.h network.h node.h system.h
pps-client-substr.o: pps-client-substr.c client.h error.h network.h node.h system.h
pps-dump-node.o: pps-dump-node.c client.h error.h node.h config.h system.h
pps-launch-server.o: pps-launch-server.c config.h hashtable.h system.h
pps-list-nodes.o: pps-list-nodes.c client.h config.h error.h system.h
test-hashtable.o: test-hashtable.c hashtable.h tests.h
test-servers.o: test-servers.c client.h error.h network.h node.h system.h
test-parser.o: test-parser.c
week04.o: week04.c hashtable.h tests.h

# Linking compiled files
pps-client-cat: pps-client-cat.o args.o client.o error.o hashtable.o network.o node.o node_list.o ring.o system.o util.o
pps-client-find: pps-client-find.o args.o client.o hashtable.o network.o system.o node.o node_list.o ring.o system.o util.o
pps-client-get: pps-client-get.o args.o client.o hashtable.o network.o system.o node.o node_list.o ring.o system.o util.o
pps-client-put: pps-client-put.o args.o client.o hashtable.o network.o system.o node.o node_list.o ring.o system.o util.o
pps-client-substr: pps-client-substr.o args.o client.o hashtable.o network.o system.o node.o node_list.o ring.o system.o util.o
pps-dump-node: pps-dump-node.o args.o client.o node.o node_list.o ring.o system.o util.o
pps-launch-server: pps-launch-server.o system.o hashtable.o
pps-list-nodes: pps-list-nodes.o args.o client.o node.o node_list.o ring.o system.o util.o
test-hashtable: test-hashtable.o hashtable.o
test-servers: test-servers.o args.o client.o hashtable.o network.o system.o node.o node_list.o ring.o system.o util.o
test-parser: test-parser.o
week04: week04.o hashtable.o

# External Makefiles
log-packets.so: Makefile.log-packets
	make -f $^


# Clean
clean:
	rm *.o $(EXEC)

