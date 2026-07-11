PREFIX  = arm-none-eabi-
CC      = $(PREFIX)gcc
CP      = $(PREFIX)objcopy
SZ      = $(PREFIX)size

TARGET    = stm32_sender
BUILD_DIR = build

# Detect OS and shell to support cross-platform commands (Windows, Linux, macOS)
ifeq ($(OS),Windows_NT)
    SHELL = cmd.exe
    RM = rmdir /s /q
    MKDIR = mkdir
else
    RM = rm -rf
    MKDIR = mkdir -p
endif

C_SOURCES = \
application/main.c \
application/app.c \
application/encryption.c \
drivers/gpio/gpio.c \
drivers/rcc/rcc.c \
drivers/uart/uart.c \
drivers/systick/systick.c \
drivers/watchdog/watchdog.c \
drivers/nvic/nvic.c \
crypto/tinyjambu.c \
crypto/tinyjambu_permutation.c \
protocol/uart_frame.c \
protocol/crc16.c \
startup/startup_stm32f103c8t6.c


C_INCLUDES = \
-Iinclude \
-Iapplication \
-Idrivers \
-Idrivers/gpio \
-Idrivers/rcc \
-Idrivers/uart \
-Idrivers/systick \
-Idrivers/watchdog \
-Idrivers/nvic \
-Icrypto \
-Iprotocol \
-Icmsis/core \
-Icmsis/device \
-Istartup

MCU = -mcpu=cortex-m3 -mthumb
CFLAGS = $(MCU) $(C_INCLUDES) -DSTM32F103xB -O2 -Wall -fdata-sections -ffunction-sections -g
LDSCRIPT = startup/linker.ld
LDFLAGS  = $(MCU) -T$(LDSCRIPT) -nostartfiles --specs=nano.specs -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(CP) -O binary $< $@

$(BUILD_DIR):
	$(MKDIR) $@

clean:
	-$(RM) $(BUILD_DIR)

.PHONY: all clean

