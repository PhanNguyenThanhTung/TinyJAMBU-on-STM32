# TinyJAMBU on STM32

Hệ thống truyền telemetry có mã hóa và xác thực giữa **STM32F103C8T6 bare-metal** và **ESP32-S3** qua UART.

## Kiến trúc

```text
STM32F103                       ESP32-S3
-----------                     --------
Telemetry payload               UART stream parser
      |                                |
TinyJAMBU-128 AEAD  -------->   CRC16 verification
      |                         TinyJAMBU tag verification
UART frame + CRC16              Replay check
                                Telemetry log
```

Frame truyền trên dây:

```text
[A5][LEN_H][LEN_L][TYPE][NONCE 12B][CIPHERTEXT N B][TAG 8B][CRC16 2B]
```

Header bốn byte là Associated Data của TinyJAMBU. CRC16 phát hiện lỗi đường truyền; authentication tag bảo vệ tính xác thực và toàn vẹn.

## Phần cứng

| STM32 Blue Pill | ESP32-S3 | Ghi chú |
|---|---|---|
| PA9 / USART1 TX | GPIO18 / UART1 RX | Dữ liệu STM32 → ESP32 |
| PA10 / USART1 RX | GPIO17 / UART1 TX | Tùy chọn, dành cho ACK sau này |
| GND | GND | Bắt buộc nối mass chung |

Cả hai MCU dùng mức logic 3.3 V. Không cấp 5 V trực tiếp vào GPIO.

## Build và test

STM32:

```bash
make -C stm32_sender clean
make -C stm32_sender
```

Host unit tests (chạy trong MSYS2 hoặc Linux có GCC):

```bash
make -C stm32_sender/tests clean all
```

ESP32-S3:

```bash
idf.py -C esp32_client set-target esp32s3
idf.py -C esp32_client build
idf.py -C esp32_client -p <PORT> flash monitor
```

## Bảo mật và nonce

- TinyJAMBU-128: key 16 byte, nonce 12 byte, tag 8 byte.
- Nonce có dạng `device_id || boot_epoch || message_counter`.
- STM32 dành hai page flash cuối (`0x0800F800`–`0x0800FFFF`) làm journal epoch chống lặp nonce qua reset/mất điện.
- Key trong `common/shared_config.h` chỉ là **key demo**. Phải thay và provision an toàn trước khi triển khai thật.
- Khi nạp firmware bằng công cụ xóa toàn chip, journal nonce cũng bị xóa. Sau mỗi lần như vậy phải dùng key mới hoặc khôi phục epoch đã lưu.
- Linker hiện giả định STM32F103C8T6 có 64 KiB flash. Nếu board clone báo dung lượng khác, phải xác minh part number và chỉnh memory map trước khi flash.

## Trạng thái

- STM32 bare-metal drivers: RCC, GPIO, SysTick, UART, NVIC, IWDG.
- TinyJAMBU-128 đã kiểm tra bằng official Known Answer Tests.
- UART framing, CRC16 và kiểm thử tamper đã hoàn thành.
- STM32 gửi telemetry mỗi giây; ESP32-S3 nhận, xác thực, chống replay và log dữ liệu.
- Bước còn lại phụ thuộc phần cứng: flash hai board và chạy kiểm thử end-to-end.
