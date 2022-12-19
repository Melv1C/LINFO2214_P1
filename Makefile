# You can use clang if you prefer
CC = gcc -pthread -lpthread -g

# Feel free to add other C flags   
CFLAGS += -c -std=gnu99 -O2 -mno-sse2 -mno-avx -mno-avx2 -mno-avx512f -fno-unroll-loops -fno-tree-vectorize -O2 -lm -march=native -mavx
# By default, we colorize the output, but this might be ugly in log files, so feel free to remove the following line.
CFLAGS += -D_COLOR

# Adapt these as you want to fit with your project
CLIENT_SOURCES = $(wildcard src/client.c)
CLIENT_GRAPH_SOURCES = $(wildcard src/client-graph.c)
TEST_CLIENT_SOURCES = $(wildcard src/test_client.c)
SERVER_SOURCES = $(wildcard src/server.c)
SERVER_OPTIM_SOURCES = $(wildcard src/server-optim.c)
SERVER_FLOAT_SOURCES = $(wildcard src/server-float.c)
SERVER_AVX_SOURCES = $(wildcard src/server-float-avx.c)


CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)
CLIENT_GRAPH_OBJECTS = $(CLIENT_GRAPH_SOURCES:.c=.o)
TEST_CLIENT_OBJECTS = $(TEST_CLIENT_SOURCES:.c=.o)
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
SERVER_OPTIM_OBJECTS = $(SERVER_OPTIM_SOURCES:.c=.o)
SERVER_FLOAT_OBJECTS = $(SERVER_FLOAT_SOURCES:.c=.o)
SERVER_AVX_OBJECTS = $(SERVER_AVX_SOURCES:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

test-client: $(TEST_CLIENT_OBJECTS)
	$(CC) $(TEST_CLIENT_OBJECTS) -o $@ $(LDFLAGS)

client: $(CLIENT_OBJECTS)
	$(CC) $(CLIENT_OBJECTS) -o $@ $(LDFLAGS)

client-graph: $(CLIENT_GRAPH_OBJECTS)
	$(CC) $(CLIENT_GRAPH_OBJECTS) -o $@ $(LDFLAGS)

server: $(SERVER_OBJECTS)
	$(CC) $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

server-optim: $(SERVER_OPTIM_OBJECTS)
	$(CC) $(SERVER_OPTIM_OBJECTS) -o $@ $(LDFLAGS)

server-float: $(SERVER_FLOAT_OBJECTS)
	$(CC) $(SERVER_FLOAT_OBJECTS) -o $@ $(LDFLAGS)

server-float-avx: $(SERVER_AVX_OBJECTS)
	$(CC) $(SERVER_AVX_OBJECTS) -o $@ $(LDFLAGS)

graph: client-graph server-float server-float-avx
	./tests/graph.sh

all: client server server-optim test-client server-float server-float-avx client-graph

.PHONY: clean mrproper

clean:
	rm -f $(CLIENT_OBJECTS) $(SERVER_OBJECTS) $(SERVER_OPTIM_OBJECTS) $(TEST_CLIENT_OBJECTS) $(SERVER_FLOAT_OBJECTS) $(SERVER_AVX_OBJECTS) $(CLIENT_GRAPH_OBJECTS)

mrproper:
	rm -f client server server-optim test-client server-float server-float-avx client-graph
	rm -f data.txt graph.png
# It is likely that you will need to update this
tests: all
	./tests/run_tests.sh

# By default, logs are disabled. But you can enable them with the debug target.
debug: CFLAGS += -D_DEBUG
debug: clean all

# Place the zip in the parent repository of the project
# ZIP_NAME="../projet2_claes_sirjacobs.zip"
ZIP_NAME="../projet3_claes.zip"

# A zip target, to help you have a proper zip file. You probably need to adapt this code.
zip:
	# Generate the log file stat now. Try to keep the repository clean.
	zip -r $(ZIP_NAME) Makefile src graph.py

TAR_NAME="../projet3_claes.tar.gz"

tar:
	# Generate the log file stat now. Try to keep the repository clean.
	tar -czvf $(TAR_NAME) Makefile src tests