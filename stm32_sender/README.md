# STM32 Sender

Firmware bare-metal cho STM32F103C8T6 (Blue Pill), chạy SYSCLK 36 MHz và gửi frame TinyJAMBU qua USART1 115200 8N1.

## Chức năng

- RCC: HSE 8 MHz → PLL 36 MHz.
- SysTick: timebase 1 ms.
- UART1: PA9 TX, PA10 RX.
- LED PC13 active-low báo mỗi lần gửi thành công.
- IWDG timeout khoảng 2 giây.
- Telemetry 10 byte: `version | flags | sequence_be32 | uptime_ms_be32`.
- Nonce: `device_id_be32 | boot_epoch_be32 | message_counter_be32`.

Hai page flash cuối được dành cho nonce epoch journal; linker chỉ cho application dùng 62 KiB đầu.

## Build

```bash
make clean
make
```

Output:

- `build/stm32_sender.elf`
- `build/stm32_sender.bin`
- `build/stm32_sender.map`

## Host tests

Từ thư mục repository:

```bash
make -C stm32_sender/tests clean all
```

Test bao gồm CRC-16/CCITT-FALSE check value, TinyJAMBU official KAT/permutation vectors, round-trip frame và tamper rejection.

## Flash

Ví dụ với ST-Link CLI/OpenOCD tùy môi trường. Không dùng mass erase với cùng production key nếu cần giữ tính duy nhất của nonce journal.
