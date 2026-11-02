# Existing CSV checks

**English** | [Tiếng Việt](CSV_CHECK.md)

These checks read saved files without opening a COM port or collecting new data. CRC bytes are not stored in CSV, so CRC error counts cannot be re-established from this table.

| File | Frames | Sequence gaps | Timestamp rate (Hz) |
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

Timestamp rate = (N-1) / sum of timestamp intervals, accounting for uint32 wrap. Timestamps measure DMA completion, not timer-edge jitter. SHA-256 hashes and minimum/maximum dt are in [csv_audit.csv](evidence/csv_audit.csv).
