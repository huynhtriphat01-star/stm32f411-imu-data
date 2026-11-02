import serial
import struct
import csv
import sys
import time

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM5"
DURATION = float(sys.argv[2]) if len(sys.argv) > 2 else 10.0

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


ser = serial.Serial(
    port=PORT,
    baudrate=115200,
    timeout=0.1
)

ser.reset_input_buffer()

filename = time.strftime("imu_%Y%m%d_%H%M%S.csv")

good_frames = 0
crc_errors = 0
lost_frames = 0

last_seq = None
buffer = bytearray()

start = time.time()
last_print = start

with open(filename, "w", newline="") as f:
    writer = csv.writer(f)

    writer.writerow([
        "pc_time",
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

    print(f"PORT     : {PORT}")
    print(f"DURATION : {DURATION:.1f} s")
    print(f"OUTPUT   : {filename}")
    print("----------------------------------------")

    while time.time() - start < DURATION:
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
            ) = struct.unpack("<HIhhhhhh", frame[2:20])

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

            if now - last_print >= 0.5:
                elapsed = now - start
                rate = good_frames / elapsed

                print(
                    f"seq={seq:5d} | "
                    f"t={timestamp_us:10d} us | "
                    f"A=({ax_g:+.3f}, {ay_g:+.3f}, {az_g:+.3f}) g | "
                    f"G=({gx_dps:+.1f}, {gy_dps:+.1f}, {gz_dps:+.1f}) dps | "
                    f"{rate:.1f} Hz"
                )

                last_print = now

ser.close()

elapsed = time.time() - start
rate = good_frames / elapsed if elapsed > 0 else 0

total_expected = good_frames + lost_frames

loss_percent = (
    100.0 * lost_frames / total_expected
    if total_expected > 0
    else 0.0
)

print()
print("============= RESULT =============")
print(f"Good frames : {good_frames}")
print(f"CRC errors  : {crc_errors}")
print(f"Lost frames : {lost_frames}")
print(f"Loss        : {loss_percent:.3f} %")
print(f"Rate        : {rate:.2f} Hz")
print(f"CSV         : {filename}")
print("==================================")