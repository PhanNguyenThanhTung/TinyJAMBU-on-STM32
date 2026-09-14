CC := gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
INCLUDES = -I../include -I../crypto -I../protocol
BUILD_DIR = build

CRYPTO_SOURCES = ../crypto/tinyjambu.c ../crypto/tinyjambu_permutation.c
PROTOCOL_SOURCES = ../protocol/crc16.c ../protocol/uart_frame.c

all: test_crc16 test_tinyjambu test_uart_frame

test_crc16: $(BUILD_DIR)/test_crc16
	$(BUILD_DIR)/test_crc16

test_tinyjambu: $(BUILD_DIR)/test_tinyjambu
	$(BUILD_DIR)/test_tinyjambu

test_uart_frame: $(BUILD_DIR)/test_uart_frame
	$(BUILD_DIR)/test_uart_frame

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/test_crc16: test_crc16.c ../protocol/crc16.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

$(BUILD_DIR)/test_tinyjambu: test_tinyjambu.c $(CRYPTO_SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

$(BUILD_DIR)/test_uart_frame: test_uart_frame.c $(CRYPTO_SOURCES) $(PROTOCOL_SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean test_crc16 test_tinyjambu test_uart_frame
