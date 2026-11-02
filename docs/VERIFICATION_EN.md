# Packaging verification - 2 November 2026

**English** | [Tiếng Việt](VERIFICATION.md)

The following records describe checks performed during preparation; hardware was not rerun for translation.

- The 15 original CSV files retain their SHA-256 hashes compared with the pre-edit backup.
- Included: 15 retrospective metadata JSONs, 15 quality JSONs, 15 quality plots and 15 PSD plots.
- Python scripts parsed successfully; quality and PSD tools were previously run on all 15 sessions.
- Every source file referenced by the Keil project exists.
- Internal README and report links point to existing files.
- Refusal to overwrite existing idle/01 was previously tested without opening a serial device.
- Both Python CRC functions were checked with `123456789 = F4`.
- Saved build evidence remains under `docs/evidence/`; no new firmware build or flash is claimed.
- G1-G3, axis-direction confirmation, collector details and acceptance of the alternative architecture remain outstanding; see [report](../report_EN.md).

## Final structure

- Two benchmark CSVs were moved into `docs/evidence/benchmark/`; the Vietnamese PDF is `docs/LAB1_Huong_dan.pdf`.
- Those three files retained their hashes during the original move. The PDF was subsequently refreshed with direct crops.
- `logger.py` still creates new CSVs in the current directory. Historical data have been organized separately.

## Current bilingual submission

- 14 screenshots are direct crops; the hardware photo retains the full frame. No text or numbers are regenerated.
- All 15 presentation images have application captions in both READMEs and PDFs. [SCREENSHOT_CROPS.csv](SCREENSHOT_CROPS.csv) records crop coordinates and original hashes.
- Vietnamese PDF: [LAB1_Huong_dan.pdf](LAB1_Huong_dan.pdf). English PDF: [LAB1_Guide_EN.pdf](LAB1_Guide_EN.pdf). Each has 16 pages and links to original evidence in `huynhtriphat01-star/stm32f411-imu-data`.
- Firmware, datasets, analysis results, images and all files under `docs/evidence/` retain their pre-translation content.
- Release `v1.0-lab1` provides complete Vietnamese and English ZIP packages. Their default README follows the selected language; both include both language guides and identical technical files.
- No hardware build, flash or measurement was repeated for this documentation update.
