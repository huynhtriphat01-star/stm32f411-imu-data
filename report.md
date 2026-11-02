# Báo cáo LAB01: thu thập dữ liệu IMU

[English](report_EN.md) | **Tiếng Việt**

## 1. Phạm vi thực hiện và cấu hình

Hệ thống hiện có dùng STM32F411CEU6 Black Pill, MPU6500 (WHO_AM_I yêu cầu 0x70), I2C1 DMA ở 100 kHz và USB CDC. PB8 nối SCL, PB9 nối SDA; dùng chung GND, nguồn theo định mức module và logic 3,3 V. Ảnh thực tế có tại [đây](docs/images/00_hardware_annotated.png), nhưng không đủ rõ để xác nhận từng dây hoặc điện áp.

Clock HSE 25 MHz, PLL M=25/N=336/P=4/Q=7 cho SYSCLK 84 MHz và USB 48 MHz. TIM2 PSC=8399, ARR=99 cho 100 Hz; TIM5 PSC=83, ARR=0xFFFFFFFF cho 1 µs/tick. MPU6500 dùng ±4 g, ±500 °/s, CONFIG=3, ACCEL_CONFIG2=3, SMPLRT_DIV=0. Frame 21 byte, AA55, seq uint16, timestamp uint32, sáu int16 little-endian, CRC-8 polynomial 0x07.

Khác đề mẫu: Black Pill/MPU6500/Keil/USB CDC thay NUCLEO-F411RE/MPU6050/CubeIDE/USART2 DMA; không có hàng đợi vòng theo mẫu. Timestamp được lấy ở DMA completion, sequence tăng sau xử lý frame, nên sequence liên tục không chứng minh mọi nhịp TIM2 đều được đọc. Cần giảng viên xác nhận phương án thay thế; chưa coi là đáp ứng mục firmware trong rubric.

## 2. Chất lượng dữ liệu hiện có

15 file gồm 5 lớp × 3 phiên, tổng 90,003 mẫu. Không chỉnh sửa CSV gốc. Chất lượng được tính lại từ timestamp MCU và sequence, không phải đo jitter bằng thiết bị ngoài. Các file khoảng 60 giây; số mẫu 5999–6001 do biên thời gian ghi. Không coi đây là bằng chứng các phiên độc lập khác giờ/ngày hoặc thứ tự lớp đã trộn.

| Lớp / phiên | Mẫu | Mất sequence (%) | p99 sai lệch dt (µs) | Trung bình độ lớn gia tốc (g) | Mẫu chạm biên int16 | CRC bị loại |
|---|---:|---:|---:|---:|---:|---:|---|
| circle 01 | 6000 | 0.000 | 0.0 | 1.0119 | 0 | chưa xác định |
| circle 02 | 6000 | 0.000 | 0.0 | 1.0181 | 0 | chưa xác định |
| circle 03 | 6000 | 0.000 | 0.0 | 1.0314 | 0 | chưa xác định |
| idle 01 | 6001 | 0.000 | 0.0 | 1.0086 | 0 | chưa xác định |
| idle 02 | 6001 | 0.000 | 0.0 | 1.0087 | 0 | chưa xác định |
| idle 03 | 5999 | 0.000 | 0.0 | 1.0168 | 0 | chưa xác định |
| shake 01 | 6000 | 0.000 | 0.0 | 1.0482 | 0 | chưa xác định |
| shake 02 | 5999 | 0.000 | 0.0 | 1.0900 | 0 | chưa xác định |
| shake 03 | 6000 | 0.000 | 0.0 | 1.1109 | 1 | chưa xác định |
| updown 01 | 6001 | 0.000 | 0.0 | 1.0329 | 0 | chưa xác định |
| updown 02 | 6000 | 0.000 | 0.0 | 1.0315 | 0 | chưa xác định |
| updown 03 | 6001 | 0.000 | 0.0 | 1.0189 | 0 | chưa xác định |
| wave 01 | 6000 | 0.000 | 0.0 | 1.0375 | 0 | chưa xác định |
| wave 02 | 6000 | 0.000 | 0.0 | 1.1558 | 0 | chưa xác định |
| wave 03 | 6001 | 0.000 | 0.0 | 1.0993 | 0 | chưa xác định |

Cả ba phiên idle có trung bình |a| trong 1 ± 0,05 g. Đây là kiểm tra trung bình độ lớn, chưa chứng minh hướng/dấu từng trục theo F2. `shake/session_03.csv` có một mẫu chạm biên; giữ nguyên và đánh dấu để xử lý ở LAB02. CRC của từng phiên không thể khôi phục từ CSV đã lọc. Ảnh terminal chỉ chứng minh những phiên được chụp, không đại diện toàn bộ 15 phiên.

[Bảng CSV](docs/quality/summary.csv); JSON và biểu đồ từng phiên ở `docs/quality/`. JSON cạnh CSV trong `raw/` được lập hồi cứu: thông tin người thu, vị trí, điều kiện và tính độc lập của phiên chưa biết được để null.

## 3. Thí nghiệm G1–G4

- **G1 — jitter phần cứng/phần mềm:** mới tính jitter từ timestamp đã lưu. Chưa có phép đo oscilloscope/logic analyzer và bản đối chiếu phần mềm theo đề. Không dùng jitter bằng 0 để kết luận jitter vật lý bằng 0.
- **G2 — aliasing/DLPF:** chưa có bộ thu có kiểm soát cho CONFIG=0/3/4 hoặc các tần số lấy mẫu khác nhau. Không thể suy ra thí nghiệm từ một cấu hình duy nhất.
- **G3 — băng thông UART:** phương án hiện tại dùng USB CDC; `115200` trong pyserial không chứng minh USART2 hoạt động ở baud đó. Phép tính lý thuyết của đề: 21 byte × 100 Hz = 2100 B/s; UART 8N1 ở 115200 có 11520 B/s, tải 18,23%. Chưa có đo thực nghiệm UART.
- **G4 — phổ công suất:** đã tạo 15 biểu đồ sáu trục, dùng periodogram Hann một phía, trừ trung bình mỗi trục, fs tính từ MCU. Không lọc thời gian hoặc loại mẫu. Tìm đỉnh trong 0,5–20 Hz, đồ thị hiển thị 0–50 Hz. Phân tích tách từng trục, không gộp PSD khác đơn vị.

- `circle`: tỷ lệ năng lượng dưới hoặc bằng 20 Hz trên các trục/phiên từ 94.4% đến 99.9%.
- `idle`: tỷ lệ năng lượng dưới hoặc bằng 20 Hz trên các trục/phiên từ 49.9% đến 97.7%.
- `shake`: tỷ lệ năng lượng dưới hoặc bằng 20 Hz trên các trục/phiên từ 99.6% đến 100.0%.
- `updown`: tỷ lệ năng lượng dưới hoặc bằng 20 Hz trên các trục/phiên từ 95.3% đến 99.9%.
- `wave`: tỷ lệ năng lượng dưới hoặc bằng 20 Hz trên các trục/phiên từ 98.9% đến 100.0%.

Xem [kết quả phổ](docs/psd/summary.json) và PNG ở `docs/psd/`. Phổ chỉ mô tả dữ liệu được gán nhãn; không xác nhận người thu thực hiện đúng cử chỉ hay chứng minh thí nghiệm aliasing.

## 4. Tái lập và phần còn phải bổ sung

Chạy `python -m pip install -r requirements.txt`, `python tools/check_quality.py`, `python tools/psd_check.py`, `python tools/protocol_check.py`. Chạy tại bản sao để giữ nguyên hồ sơ đã nộp. Logger phần cứng: `python logger.py COM5 10`; thay COM5 theo máy. Thu mới bằng `collect.py` chỉ khi tên phiên chưa tồn tại.

Đã bổ sung README, HTML đề bài, PDF, ảnh, công cụ, JSON hồi cứu và báo cáo. Chưa build lại hoặc flash sau đóng gói. Để tuyên bố đạt toàn bộ đề, cần hoàn tất G1–G3, xác nhận biến thể phần cứng/toolchain, bổ sung thông tin sinh viên và thông tin thu thực tế, xác minh hướng trục và tính độc lập các phiên. Không tự đổi firmware đang hoạt động sang kiến trúc khác khi chưa kiểm tra phần cứng.
