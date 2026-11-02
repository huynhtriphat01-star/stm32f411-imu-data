import serial
import struct
import csv
import sys
import time
from pathlib import Path

FRAME_SIZE = 21
SOF = b"\xAA\x55"

ACC_SCALE = 8192.0
GYRO_SCALE = 65.5


def crc8(data: bytes) -> int:
    crc = 0x00

    for byte in data:
        crc ^= byte

        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF

    return crc


def main():
    if len(sys.argv) != 5:
        print("Usage:")
        print("python collect.py COM5 idle 01 60")
        sys.exit(1)

    port = sys.argv[1]
    label = sys.argv[2]
    session = sys.argv[3]
    duration = float(sys.argv[4])

    valid_labels = {
        "idle",
        "wave",
        "circle",
        "updown",
        "shake",
    }

    if label not in valid_labels:
        print(f"Invalid label: {label}")
        print("Valid labels:", ", ".join(sorted(valid_labels)))
        sys.exit(1)

    out_dir = Path("raw") / label
    out_dir.mkdir(parents=True, exist_ok=True)

    filename = out_dir / f"session_{session}.csv"

    if filename.exists():
        raise FileExistsError(f"Refusing to overwrite existing dataset: {filename}")

    ser = serial.Serial(
        port=port,
        baudrate=115200,
        timeout=0.1
    )

    ser.reset_input_buffer()

    good_frames = 0
    crc_errors = 0
    lost_frames = 0

    last_seq = None
    buffer = bytearray()

    start = time.time()
    last_print = start

    print()
    print("==========================================")
    print("STM32 IMU DATASET COLLECTOR")
    print("==========================================")
    print(f"PORT     : {port}")
    print(f"LABEL    : {label}")
    print(f"SESSION  : {session}")
    print(f"DURATION : {duration:.1f} s")
    print(f"OUTPUT   : {filename}")
    print("------------------------------------------")
    print("START MOVEMENT NOW")
    print("------------------------------------------")

    with open(filename, "x", newline="") as f:
        writer = csv.writer(f)

        writer.writerow([
            "pc_time",
            "label",
            "session",
            "seq",
            "timestamp_us",

            "ax_raw",
            "ay_raw",
            "az_raw",

            "gx_raw",
            "gy_raw",
            "gz_raw",

            "ax_g",
            "ay_g",
            "az_g",

            "gx_dps",
            "gy_dps",
            "gz_dps"
        ])

        while time.time() - start < duration:

            data = ser.read(ser.in_waiting or 1)

            if data:
                buffer.extend(data)

            while len(buffer) >= FRAME_SIZE:

                pos = buffer.find(SOF)

                if pos < 0:
                    buffer.clear()
                    break

                if pos > 0:
                    del buffer[:pos]

                if len(buffer) < FRAME_SIZE:
                    break

                frame = bytes(buffer[:FRAME_SIZE])

                expected_crc = crc8(frame[:20])
                received_crc = frame[20]

                if expected_crc != received_crc:
                    crc_errors += 1
                    del buffer[0]
                    continue

                del buffer[:FRAME_SIZE]

                (
                    seq,
                    timestamp_us,
                    ax_raw,
                    ay_raw,
                    az_raw,
                    gx_raw,
                    gy_raw,
                    gz_raw
                ) = struct.unpack(
                    "<HIhhhhhh",
                    frame[2:20]
                )

                if last_seq is not None:
                    expected_seq = (last_seq + 1) & 0xFFFF

                    if seq != expected_seq:
                        gap = (seq - expected_seq) & 0xFFFF

                        if gap < 10000:
                            lost_frames += gap

                last_seq = seq

                ax_g = ax_raw / ACC_SCALE
                ay_g = ay_raw / ACC_SCALE
                az_g = az_raw / ACC_SCALE

                gx_dps = gx_raw / GYRO_SCALE
                gy_dps = gy_raw / GYRO_SCALE
                gz_dps = gz_raw / GYRO_SCALE

                writer.writerow([
                    time.time(),
                    label,
                    session,
                    seq,
                    timestamp_us,

                    ax_raw,
                    ay_raw,
                    az_raw,

                    gx_raw,
                    gy_raw,
                    gz_raw,

                    ax_g,
                    ay_g,
                    az_g,

                    gx_dps,
                    gy_dps,
                    gz_dps
                ])

                good_frames += 1

                now = time.time()

                if now - last_print >= 1.0:
                    elapsed = now - start
                    rate = good_frames / elapsed

                    print(
                        f"{elapsed:5.1f}/{duration:.1f}s | "
                        f"frames={good_frames:5d} | "
                        f"rate={rate:6.2f} Hz | "
                        f"CRC={crc_errors} | "
                        f"lost={lost_frames}"
                    )

                    last_print = now

    ser.close()

    elapsed = time.time() - start

    rate = good_frames / elapsed if elapsed > 0 else 0.0

    total_expected = good_frames + lost_frames

    loss_percent = (
        100.0 * lost_frames / total_expected
        if total_expected > 0
        else 0.0
    )

    print()
    print("=============== RESULT ==================")
    print(f"Label       : {label}")
    print(f"Session     : {session}")
    print(f"Good frames : {good_frames}")
    print(f"CRC errors  : {crc_errors}")
    print(f"Lost frames : {lost_frames}")
    print(f"Loss        : {loss_percent:.3f} %")
    print(f"Rate        : {rate:.2f} Hz")
    print(f"CSV         : {filename}")
    print("==========================================")


if __name__ == "__main__":
    main()