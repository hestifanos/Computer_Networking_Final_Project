CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
BUILD_DIR := build

.PHONY: all clean

all: $(BUILD_DIR)/tcp_client $(BUILD_DIR)/tcp_server

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/tcp_client: Client/TCP_Client.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/tcp_server: Server/TCP_Server.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)
