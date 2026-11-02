# Kiểm tra sau đóng gói — 02/11/2026

[English](VERIFICATION_EN.md) | **Tiếng Việt**

- 15 CSV gốc giữ nguyên SHA-256 so với bản sao lưu trước sửa.
- 15 JSON metadata hồi cứu, 15 JSON chất lượng, 15 biểu đồ chất lượng, 15 biểu đồ PSD.
- Các script Python phân tích cú pháp thành công; công cụ chất lượng và PSD đã chạy trên cả 15 phiên.
- Mọi file nguồn được tham chiếu trong project Keil đều tồn tại.
- Liên kết nội bộ README và report.md đều trỏ tới file có thật.
- Kiểm tra từ chối ghi đè đã chạy trên phiên idle/01 có sẵn, không mở thiết bị nối tiếp.
- CRC hai hàm Python được kiểm tra bằng vector 123456789 = F4.
- Bản build cũ đã được giữ dưới docs/evidence; chưa build firmware mới hoặc flash phần cứng.
- G1–G3, xác nhận hướng trục, thông tin người thu và chấp nhận kiến trúc thay thế còn thiếu; xem report.md.

## Dọn cấu trúc cuối

- Hai CSV benchmark được chuyển từ gốc sang `docs/evidence/benchmark/`; PDF chuyển sang `docs/LAB1_Huong_dan.pdf`.
- Tại bước dọn cấu trúc, nội dung ba file giữ nguyên SHA-256. PDF sau đó được làm mới bằng ảnh crop trực tiếp từ bản gốc.
- `logger.py` vẫn tạo CSV mới tại thư mục hiện tại; dữ liệu lịch sử đã được sắp xếp lại.


## Kiểm tra bản nộp hiện tại

- 14 screenshot được crop trực tiếp từ ảnh gốc; ảnh phần cứng giữ toàn bộ khung hình. Không tái tạo chữ hoặc số.
- 15 ảnh trình bày có caption phần mềm trong README và PDF. Tọa độ crop và SHA-256 nguồn nằm trong `SCREENSHOT_CROPS.csv`.
- PDF tiếng Việt `LAB1_Huong_dan.pdf` và PDF tiếng Anh `LAB1_Guide_EN.pdf` đều gồm 16 trang; liên kết ảnh gốc trỏ về repo `huynhtriphat01-star/stm32f411-imu-data`.
- Firmware, dataset, kết quả phân tích và toàn bộ `docs/evidence/` giữ nguyên nội dung so với bản đã kiểm tra.
- Bản phát hành `v1.0-lab1` có hai ZIP đầy đủ tiếng Việt và tiếng Anh. README mặc định theo ngôn ngữ; cả hai đều chứa hướng dẫn hai ngôn ngữ và cùng nội dung kỹ thuật.
- Không build, flash hoặc đo lại phần cứng trong lần đóng gói này.
