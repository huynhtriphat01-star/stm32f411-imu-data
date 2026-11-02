# Kiểm tra CSV hiện có

[English](CSV_CHECK_EN.md) | **Tiếng Việt**

Đọc file lưu sẵn; không kết nối COM hoặc thu lại. CRC không được lưu trong CSV, nên không tái xác minh CRC errors từ bảng này.

| File | Frames | Sequence gián đoạn | Rate timestamp (Hz) |
|---|---:|---:|---:|
| raw/circle/session_01.csv | 6000 | 0 | 100.0 |
| raw/circle/session_02.csv | 6000 | 0 | 100.0 |
| raw/circle/session_03.csv | 6000 | 0 | 100.0 |
| raw/idle/session_01.csv | 6001 | 0 | 100.0 |
| raw/idle/session_02.csv | 6001 | 0 | 100.0 |
| raw/idle/session_03.csv | 5999 | 0 | 100.0 |
| raw/shake/session_01.csv | 6000 | 0 | 100.0 |
| raw/shake/session_02.csv | 5999 | 0 | 100.0 |
| raw/shake/session_03.csv | 6000 | 0 | 100.0 |
| raw/updown/session_01.csv | 6001 | 0 | 100.0 |
| raw/updown/session_02.csv | 6000 | 0 | 100.0 |
| raw/updown/session_03.csv | 6001 | 0 | 100.0 |
| raw/wave/session_01.csv | 6000 | 0 | 100.0 |
| raw/wave/session_02.csv | 6000 | 0 | 100.0 |
| raw/wave/session_03.csv | 6001 | 0 | 100.0 |
| [imu_20261101_145817.csv](evidence/benchmark/imu_20261101_145817.csv) | 1000 | 0 | 100.00008 |
| [imu_20261101_173954.csv](evidence/benchmark/imu_20261101_173954.csv) | 1000 | 0 | 100.0 |

Rate timestamp = (N-1) / tổng khoảng timestamp (có xử lý uint32 wrap). Khoảng timestamp đo thời điểm DMA hoàn tất; không tương đương jitter cạnh timer. SHA-256 và min/max dt nằm trong csv_audit.csv.