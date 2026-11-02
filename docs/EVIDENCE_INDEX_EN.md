# LAB1 evidence index

[English](EVIDENCE_INDEX_EN.md) | [Tiếng Việt](EVIDENCE_INDEX.md)

Source: user-supplied `1 11.zip`, containing 151 PNGs. All were screened in a contact sheet; candidate originals and questionable settings were inspected individually. The guide uses 14 screenshots and excludes 137. No duplicate SHA-256 hashes were found, but many images show similar content. One source image is only 112 x 2 pixels.

Source images are preserved. Individual decisions remain in the original Vietnamese [SCREENSHOT_REVIEW.csv](evidence/SCREENSHOT_REVIEW.csv).

Presentation images are direct crops without regenerated UI, text or numbers. README and PDF captions name the main application. Original files and their SHA-256 hashes remain unchanged. Crop coordinates and source hashes are listed in [SCREENSHOT_CROPS.csv](SCREENSHOT_CROPS.csv).

| Order | Content | Capture ID | Presentation | Original |
|---|---|---|---|---|
| 01 | Identify the MPU6500 | 124400 | [View](images/14_whoami_annotated.png) | [Original](evidence/14_whoami_original.png) |
| 02 | SYSCLK 84 MHz and USB 48 MHz | 141948 | [View](images/01_clock_annotated.png) | [Original](evidence/01_clock_original.png) |
| 03 | TIM2 sampling at 100 Hz | 142134 | [View](images/02_tim2_annotated.png) | [Original](evidence/02_tim2_original.png) |
| 04 | TIM2 interrupt | 134416 | [View](images/12_tim2_irq_annotated.png) | [Original](evidence/12_tim2_irq_original.png) |
| 05 | TIM5 timestamps at 1 us | 142218 | [View](images/03_tim5_annotated.png) | [Original](evidence/03_tim5_original.png) |
| 06 | I2C1 RX DMA | 134754 | [View](images/04_dma_annotated.png) | [Original](evidence/04_dma_original.png) |
| 07 | NVIC for I2C and DMA | 134746 | [View](images/05_nvic_annotated.png) | [Original](evidence/05_nvic_original.png) |
| 08 | USB OTG FS Device Only | 142247 | [View](images/13_usb_mode_annotated.png) | [Original](evidence/13_usb_mode_original.png) |
| 09 | USB_DEVICE CDC | 142830 | [View](images/06_cdc_annotated.png) | [Original](evidence/06_cdc_original.png) |
| 10 | Keil build and Flash Verify OK | 145426 | [View](images/07_build_flash_annotated.png) | [Original](evidence/07_build_flash_original.png) |
| 11 | Windows detects COM5 | 173501 | [View](images/08_com5_annotated.png) | [Original](evidence/08_com5_original.png) |
| 12 | Python benchmark for 10 s | 145837 | [View](images/09_benchmark_annotated.png) | [Original](evidence/09_benchmark_original.png) |
| 13 | Collect by class and session | 174639 | [View](images/10_collector_annotated.png) | [Original](evidence/10_collector_original.png) |
| 14 | Complete all 15 CSV files | 181343 | [View](images/11_dataset_complete_annotated.png) | [Original](evidence/11_dataset_complete_original.png) |

## Coverage and remaining evidence gaps

The initial ZIP had no hardware photo; the user later supplied one separately. Images illustrate clock settings, TIM2, TIM5, DMA/NVIC, USB CDC, build, flash, COM5, benchmark and dataset completion. DMA Channel 1, the 21-byte frame and CRC-8 polynomial 0x07 were checked against source code; captions do not claim these appear in CubeMX screenshots when they are not visible.

## Intermediate settings to avoid

- 141802: PLLQ=4 and USB branch 84 MHz. Use 141948: PLLQ=7, USB 48 MHz.
- 131633: TIM2 PSC=15999/ARR=9. Use 142134: PSC=8399/ARR=99 at timer clock 84 MHz.
- 135940: TIM5 PSC=15. Use 142218: PSC=83.
- 134600: DMA priority Low. Use 134754: High.
- 134703: I2C event/error IRQs missing. Use 134746: all three IRQs enabled.
- 142709: USB_DEVICE disabled. Use 142830: CDC.
- 175443/175653: incomplete or failed collection attempts; not proof of success.
- 181140/181941: updown 03 still missing. Use 181343 and the CSV audit for the completed dataset.

## Provenance and confidence

Screenshot 145426 directly shows build success and Verify OK. Screenshot 145837 shows 1000 frames/0 CRC/0 lost/99.96 Hz; 174012 is a different run at 99.92 Hz. Screenshot 181343 shows 6001 frames/0 CRC/0 lost/99.99 Hz and a total count of 15 CSVs. Three sessions for each of five classes were established from the files, not inferred from a screenshot listing only updown.

## Presentation method

Earlier AI-annotated images have been replaced by deterministic crops of originals. No arrows or labels cover evidence. Existing filenames are retained for link compatibility. The hardware photo retains its entire frame and is not a software screenshot. English and Vietnamese guides share these unchanged crops; translations appear in captions, not inside the evidence.

## Hardware photo

The user-supplied photo shows an IMU module, STM32 board, breadboard and jumper wires. It does not verify individual pins, supply voltage or wire colors. PB8=SCL and PB9=SDA come from source configuration, not visual wire tracing.

[Presentation](images/00_hardware_annotated.png) | [Original](evidence/00_hardware_original.jpg)

SHA-256: `f218e431b5db0caeb0e6e91f83b28a33ea4cfacf4fcc697ad16c5aa01aa4f380`
