# Sổ bằng chứng LAB1 - bản cập nhật từ ZIP

[English](EVIDENCE_INDEX_EN.md) | **Tiếng Việt**

Nguồn: `C:\Users\LENOVO\Pictures\Screenshots\1 11.zip`, 151 PNG. Rà toàn bộ bằng bảng ảnh thu nhỏ; mở ảnh gốc các ứng viên và ảnh có thông số đáng ngờ để chọn. Giữ 14 ảnh, không đưa 137 ảnh vào hướng dẫn. Không có file trùng SHA-256; nhiều ảnh gần lặp về nội dung. Một ảnh chỉ 112 x 2 pixel.

Ảnh cũ trong chat nay đã được bổ sung bằng ZIP người dùng cung cấp. Không xóa ảnh nguồn. Chi tiết quyết định cho từng ảnh: [SCREENSHOT_REVIEW.csv](evidence/SCREENSHOT_REVIEW.csv).

Bản trình bày hiện tại được crop trực tiếp từ ảnh gốc, không tái tạo giao diện, chữ hoặc số. Caption trong README và PDF ghi đúng phần mềm chính. Ảnh gốc và các SHA-256 trong hồ sơ bằng chứng được giữ nguyên. Tọa độ crop và SHA-256 nguồn được ghi tại [SCREENSHOT_CROPS.csv](SCREENSHOT_CROPS.csv).

| Thứ tự | Nội dung | Ảnh nguồn | Chú thích | Ảnh gốc |
|---|---|---|---|---|
| 01 | Nhận diện MPU6500 | 124400 | [Xem](images/14_whoami_annotated.png) | [Gốc](evidence/14_whoami_original.png) |
| 02 | Clock 84 MHz và USB 48 MHz | 141948 | [Xem](images/01_clock_annotated.png) | [Gốc](evidence/01_clock_original.png) |
| 03 | TIM2 lấy mẫu 100 Hz | 142134 | [Xem](images/02_tim2_annotated.png) | [Gốc](evidence/02_tim2_original.png) |
| 04 | Ngắt TIM2 | 134416 | [Xem](images/12_tim2_irq_annotated.png) | [Gốc](evidence/12_tim2_irq_original.png) |
| 05 | TIM5 timestamp 1 us | 142218 | [Xem](images/03_tim5_annotated.png) | [Gốc](evidence/03_tim5_original.png) |
| 06 | I2C1 RX DMA | 134754 | [Xem](images/04_dma_annotated.png) | [Gốc](evidence/04_dma_original.png) |
| 07 | NVIC cho I2C và DMA | 134746 | [Xem](images/05_nvic_annotated.png) | [Gốc](evidence/05_nvic_original.png) |
| 08 | USB OTG FS Device Only | 142247 | [Xem](images/13_usb_mode_annotated.png) | [Gốc](evidence/13_usb_mode_original.png) |
| 09 | USB_DEVICE CDC | 142830 | [Xem](images/06_cdc_annotated.png) | [Gốc](evidence/06_cdc_original.png) |
| 10 | Keil build và Flash Verify OK | 145426 | [Xem](images/07_build_flash_annotated.png) | [Gốc](evidence/07_build_flash_original.png) |
| 11 | Windows nhận COM5 | 173501 | [Xem](images/08_com5_annotated.png) | [Gốc](evidence/08_com5_original.png) |
| 12 | Benchmark Python 10 s | 145837 | [Xem](images/09_benchmark_annotated.png) | [Gốc](evidence/09_benchmark_original.png) |
| 13 | Thu dataset theo class và session | 174639 | [Xem](images/10_collector_annotated.png) | [Gốc](evidence/10_collector_original.png) |
| 14 | Hoàn tất 15 CSV | 181343 | [Xem](images/11_dataset_complete_annotated.png) | [Gốc](evidence/11_dataset_complete_original.png) |

## Những bước còn thiếu ảnh

ZIP ban đầu không có ảnh phần cứng; nay đã bổ sung ảnh thực tế người dùng gửi riêng. Bộ ảnh đã đủ minh họa clock, TIM2, TIM5, DMA/NVIC, USB CDC, build, flash, COM5, benchmark và dataset. Channel 1 của DMA, frame 21 byte và CRC-8 poly 0x07 được đối chiếu từ source; không gắn nhãn rằng các giá trị này xuất hiện trên ảnh CubeMX khi ảnh không hiển thị.

## Các cấu hình trung gian cần tránh

- 141802: PLLQ=4 và nhánh USB 84 MHz; dùng 141948: PLLQ=7 và USB 48 MHz.
- 131633: TIM2 PSC=15999/ARR=9; dùng 142134: PSC=8399/ARR=99 tại timer clock 84 MHz.
- 135940: TIM5 PSC=15; dùng 142218: PSC=83.
- 134600: DMA priority Low; dùng 134754: High.
- 134703: thiếu I2C event/error IRQ; dùng 134746: đã bật cả ba IRQ.
- 142709: USB_DEVICE Disable; dùng 142830: CDC.
- 175443/175653: lượt thu chưa hoàn tất hoặc có lỗi; không dùng chứng minh kết quả thành công.
- 181140/181941: còn thiếu updown 03; dùng ảnh 181343 và CSV audit để chứng minh đủ bộ.

## Ghi nhận nguồn và độ tin cậy

Ảnh 145426 trực tiếp chứng minh build và Verify OK. Ảnh 145837 trực tiếp chứng minh benchmark 1000 frames/0 CRC/0 lost/99.96 Hz; ảnh 174012 là lượt kiểm tra khác đạt 99.92 Hz. Ảnh 181343 trực tiếp chứng minh 6001 frames/0 CRC/0 lost/99.99 Hz và tổng 15 CSV. Việc đủ 3 session cho cả 5 class được xác minh bằng file hiện có, không suy ra từ ảnh chỉ liệt kê updown.

## Phương pháp trình bày

Các bản chú thích AI trước đây đã được thay bằng crop xác định từ ảnh gốc. Không thêm mũi tên hoặc label đè lên bằng chứng. Giữ tên file để các liên kết hiện có tiếp tục hoạt động. Ảnh phần cứng giữ toàn bộ khung hình; đây không phải screenshot phần mềm.

## Ảnh phần cứng bổ sung

Ảnh thực tế do người dùng cung cấp: module IMU, board STM32 trên breadboard và dây nối thử nghiệm. Ảnh chưa đủ rõ để xác nhận từng chân, nguồn cấp hoặc ánh xạ màu dây. Bảng PB8=SCL, PB9=SDA là cấu hình từ source, không phải kết luận truy dây từ ảnh.

[Chú thích tiếng Việt](images/00_hardware_annotated.png) | [Ảnh gốc](evidence/00_hardware_original.jpg)

SHA-256: `f218e431b5db0caeb0e6e91f83b28a33ea4cfacf4fcc697ad16c5aa01aa4f380`
