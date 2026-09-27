# ESP32-S3 TinyJAMBU Receiver

ESP-IDF application nhận frame từ STM32 qua UART1, kiểm tra CRC16, xác thực/giải mã TinyJAMBU-128 và loại frame replay.

## Wiring mặc định

- STM32 PA9 (TX) → ESP32-S3 GPIO18 (RX).
- ESP32-S3 GPIO17 (TX) → STM32 PA10 (RX), tùy chọn.
- Nối chung GND.

Có thể đổi pin trong `main/app_main.h` nếu board ESP32-S3 của bạn dùng chân khác.

## Build và flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash monitor
```

Khi khởi động thành công, monitor hiển thị:

```text
TinyJAMBU UART receiver ready
```

Mỗi frame hợp lệ sẽ in sequence và uptime. Frame sai CRC, sai tag, sai device ID hoặc nonce cũ sẽ bị từ chối.

## Lưu ý

Replay state hiện lưu trong RAM của ESP32. TinyJAMBU vẫn ngăn sửa/giả mạo frame, nhưng để chống replay xuyên qua reset ESP32 trong production, hãy lưu `(epoch, counter)` trong NVS.
