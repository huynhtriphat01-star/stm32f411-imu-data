# LAB01 report: IMU data acquisition

**English** | [Tiếng Việt](report.md)

## 1. Implementation scope and configuration

The system uses STM32F411CEU6 Black Pill, MPU6500 (`WHO_AM_I` expected to be `0x70`), I2C1 DMA at 100 kHz and USB CDC. PB8 connects to SCL and PB9 to SDA. Use a common ground, a module-compatible supply and 3.3 V logic. The [assembly photo](docs/images/00_hardware_annotated.png) does not resolve individual wires or establish voltage.

HSE is 25 MHz; PLL M=25/N=336/P=4/Q=7 gives SYSCLK 84 MHz and USB 48 MHz. TIM2 PSC=8399, ARR=99 gives 100 Hz. TIM5 PSC=83, ARR=0xFFFFFFFF gives 1 µs/tick. MPU6500 uses ±4 g, ±500 °/s, CONFIG=3, ACCEL_CONFIG2=3 and SMPLRT_DIV=0. Frames contain 21 bytes: AA55, uint16 sequence, uint32 timestamp, six little-endian int16 axes and CRC-8 polynomial 0x07.

Differences from the reference assignment: Black Pill/MPU6500/Keil/USB CDC replace NUCLEO-F411RE/MPU6050/CubeIDE/USART2 DMA. The reference ring buffer is absent. Timestamps are captured at DMA completion and sequence increments after frame processing; continuous sequences do not prove every TIM2 sample request was serviced. Instructor acceptance of the substitutions is required; firmware-rubric compliance is not established.

## 2. Quality of existing data

There are 15 files across 5 classes × 3 sessions, totaling 90,003 samples. Original CSV files were not edited. Quality was recalculated from MCU timestamps and sequence numbers, not from external jitter measurements. Sessions last approximately 60 seconds, with 5999-6001 samples due to recording boundaries. This does not establish collection on independent dates/times or randomized class order.

| Class / session | Samples | Sequence loss (%) | p99 dt deviation (µs) | Mean acceleration magnitude (g) | Samples reaching int16 limits | Rejected CRC frames |
|---|---:|---:|---:|---:|---:|---|
| circle 01 | 6000 | 0.000 | 0.0 | 1.0119 | 0 | unknown |
| circle 02 | 6000 | 0.000 | 0.0 | 1.0181 | 0 | unknown |
| circle 03 | 6000 | 0.000 | 0.0 | 1.0314 | 0 | unknown |
| idle 01 | 6001 | 0.000 | 0.0 | 1.0086 | 0 | unknown |
| idle 02 | 6001 | 0.000 | 0.0 | 1.0087 | 0 | unknown |
| idle 03 | 5999 | 0.000 | 0.0 | 1.0168 | 0 | unknown |
| shake 01 | 6000 | 0.000 | 0.0 | 1.0482 | 0 | unknown |
| shake 02 | 5999 | 0.000 | 0.0 | 1.0900 | 0 | unknown |
| shake 03 | 6000 | 0.000 | 0.0 | 1.1109 | 1 | unknown |
| updown 01 | 6001 | 0.000 | 0.0 | 1.0329 | 0 | unknown |
| updown 02 | 6000 | 0.000 | 0.0 | 1.0315 | 0 | unknown |
| updown 03 | 6001 | 0.000 | 0.0 | 1.0189 | 0 | unknown |
| wave 01 | 6000 | 0.000 | 0.0 | 1.0375 | 0 | unknown |
| wave 02 | 6000 | 0.000 | 0.0 | 1.1558 | 0 | unknown |
| wave 03 | 6001 | 0.000 | 0.0 | 1.0993 | 0 | unknown |

All three idle sessions have mean |a| within 1 ± 0.05 g. This checks mean magnitude, not each axis direction/sign required by F2. `shake/session_03.csv` contains one sample at an int16 limit; it is retained and flagged for LAB02. Rejected CRC counts cannot be recovered from filtered CSVs. Terminal screenshots establish only the captured sessions, not all 15.

[CSV summary](docs/quality/summary.csv); per-session JSONs and plots are in `docs/quality/`. Metadata JSONs beside the CSVs in `raw/` were created retrospectively. Unknown collector identity, location, conditions and session independence remain null.

## 3. Experiments G1-G4

- **G1 - hardware/software jitter:** only saved-timestamp jitter has been computed. No oscilloscope/logic-analyzer measurement or required hardware/software comparison is available. Zero timestamp jitter does not establish zero physical jitter.
- **G2 - aliasing/DLPF:** no controlled recordings compare CONFIG=0/3/4 or different sampling frequencies. A single configuration does not establish this experiment.
- **G3 - UART bandwidth:** the implementation uses USB CDC. A pyserial setting of `115200` does not prove USART2 runs at that baud rate. Reference calculation: 21 bytes × 100 Hz = 2100 B/s; UART 8N1 at 115200 provides 11520 B/s, for 18.23% utilization. No experimental UART measurement is available.
- **G4 - power spectrum:** 15 six-axis plots were generated using a one-sided Hann periodogram, subtracting each axis mean and estimating fs from MCU timestamps. No time-domain filtering or sample removal was applied. Peaks are searched within 0.5-20 Hz; plots display 0-50 Hz. Axes are analyzed separately; PSDs with different units are not combined.

- `circle`: energy at or below 20 Hz ranges from 94.4% to 99.9% across axes/sessions.
- `idle`: energy at or below 20 Hz ranges from 49.9% to 97.7% across axes/sessions.
- `shake`: energy at or below 20 Hz ranges from 99.6% to 100.0% across axes/sessions.
- `updown`: energy at or below 20 Hz ranges from 95.3% to 99.9% across axes/sessions.
- `wave`: energy at or below 20 Hz ranges from 98.9% to 100.0% across axes/sessions.

See [spectrum results](docs/psd/summary.json) and PNGs in `docs/psd/`. Spectra describe labeled data; they do not verify the collector's gestures or establish an aliasing experiment.

## 4. Reproduction and outstanding work

Run `python -m pip install -r requirements.txt`, `python tools/check_quality.py`, `python tools/psd_check.py` and `python tools/protocol_check.py`. Work on a copy to preserve submitted artifacts. Hardware logger: `python logger.py COM5 10`; replace COM5 with the assigned port. Use `collect.py` only when the target session filename is absent.

The submission includes READMEs, original assignment HTML, both PDF guides, images, tools, retrospective metadata JSONs and reports. No new firmware build or flash was performed during packaging. Full assignment compliance still requires G1-G3, acceptance of hardware/toolchain substitutions, confirmed student and acquisition details, axis-direction checks and session independence. Do not migrate the working firmware to another architecture without hardware verification.
