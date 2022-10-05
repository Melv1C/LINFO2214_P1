# You can use clang if you prefer
CC = gcc

# Feel free to add other C flags   
CFLAGS += -c -Wall -std=gnu99 -O2 -Wextra
# By default, we colorize the output, but this might be ugly in log files, so feel free to remove the following line.
CFLAGS += -D_COLOR 

# You may want to add something here
LDFLAGS += -fPIE -lz -g 

# Adapt these as you want to fit with your project
CLIENT_SOURCES = $(wildcard src/client.c)
SERVER_SOURCES = $(wildcard src/server.c)

CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)

CLIENT = client
SERVER = server


all: $(CLIENT) $(SERVER)
$(CLIENT): $(CLIENT_OBJECTS)
	$(CC) $(CLIENT_OBJECTS) -o $@ $(LDFLAGS)

$(SERVER): $(SERVER_OBJECTS)
	$(CC) $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

.PHONY: clean mrproper

clean:
	rm -f $(CLIENT_OBJECTS) $(SERVER_OBJECTS)

mrproper:
	rm -f $(CLIENT) $(SERVER)
	rm -f 
# It is likely that you will need to update this
tests: debug
	./tests/run_tests.sh

# By default, logs are disabled. But you can enable them with the debug target.
debug: CFLAGS += -D_DEBUG
debug: clean all

# Place the zip in the parent repository of the project
ZIP_NAME="../projet1_nom1_nom2.zip"

# A zip target, to help you have a proper zip file. You probably need to adapt this code.
zip:
	# Generate the log file stat now. Try to keep the repository clean.
	git log --stat > gitlog.stat
	zip -r $(ZIP_NAME) Makefile README.md src tests rapport.pdf gitlog.stat
	# We remove it now, but you can leave it if you want.
	rm gitlog.stat
