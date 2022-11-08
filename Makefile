# You can use clang if you prefer
CC = gcc -pthread -lpthread -g

# Feel free to add other C flags   
CFLAGS += -c -std=gnu99 -O2 -mno-sse2 -mno-avx -mno-avx2 -mno-avx512f -fno-unroll-loops -fno-tree-vectorize -O2 -lm
# By default, we colorize the output, but this might be ugly in log files, so feel free to remove the following line.
CFLAGS += -D_COLOR

# Adapt these as you want to fit with your project
CLIENT_SOURCES = $(wildcard src/client.c)
TEST_CLIENT_SOURCES = $(wildcard src/test_client.c)
SERVER_SOURCES = $(wildcard src/server.c src/log.c)
SERVER_OPTIM_SOURCES = $(wildcard src/server-optim.c)
SERVER_C_L_SOURCES = $(wildcard src/server-line-line.c)
SERVER_UNROLLING_SOURCES = $(wildcard src/server-unrolling.c)
SERVER_OPTIM_MIN_SOURCES = $(wildcard src/server-optim-min.c)

CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)
TEST_CLIENT_OBJECTS = $(TEST_CLIENT_SOURCES:.c=.o)
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
SERVER_OPTIM_OBJECTS = $(SERVER_OPTIM_SOURCES:.c=.o)
SERVER_C_L_OBJECTS = $(SERVER_C_L_SOURCES:.c=.o)
SERVER_UNROLLING_OBJECTS = $(SERVER_UNROLLING_SOURCES:.c=.o)
SERVER_OPTIM_MIN_OBJECTS = $(SERVER_OPTIM_MIN_SOURCES:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

test-client: $(TEST_CLIENT_OBJECTS)
	$(CC) $(TEST_CLIENT_OBJECTS) -o $@ $(LDFLAGS)

client: $(CLIENT_OBJECTS)
	$(CC) $(CLIENT_OBJECTS) -o $@ $(LDFLAGS)

server: $(SERVER_OBJECTS)
	$(CC) $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

server-optim: $(SERVER_OPTIM_OBJECTS)
	$(CC) $(SERVER_OPTIM_OBJECTS) -o $@ $(LDFLAGS)

server-line-line: $(SERVER_C_L_OBJECTS)
	$(CC) $(SERVER_C_L_OBJECTS) -o $@ $(LDFLAGS)

server-unrolling: $(SERVER_UNROLLING_OBJECTS)
	$(CC) $(SERVER_UNROLLING_OBJECTS) -o $@ $(LDFLAGS)

server-optim-min: $(SERVER_OPTIM_MIN_OBJECTS)
	$(CC) $(SERVER_OPTIM_MIN_OBJECTS) -o $@ $(LDFLAGS)

all: client server server-optim test-client server-line-line server-unrolling

.PHONY: clean mrproper

clean:
	rm -f $(CLIENT_OBJECTS) $(SERVER_OBJECTS) $(SERVER_OPTIM_OBJECTS) $(TEST_CLIENT_OBJECTS) $(SERVER_C_L_OBJECTS) $(SERVER_UNROLLING_OBJECTS) $(SERVER_OPTIM_MIN_OBJECTS)

mrproper:
	rm -f client server server-optim test-client server-column-line server-unrolling server-optim-min
	rm -f 
# It is likely that you will need to update this
tests: all
	./tests/run_tests.sh

# By default, logs are disabled. But you can enable them with the debug target.
debug: CFLAGS += -D_DEBUG
debug: clean all

# Place the zip in the parent repository of the project
ZIP_NAME="../projet2_claes_sirjacobs.zip"

# A zip target, to help you have a proper zip file. You probably need to adapt this code.
zip:
	# Generate the log file stat now. Try to keep the repository clean.
	zip -r $(ZIP_NAME) Makefile src

TAR_NAME="../projet2_claes_sirjacobs.tar.gz"

tar:
	# Generate the log file stat now. Try to keep the repository clean.
	tar -czvf $(TAR_NAME) Makefile src

