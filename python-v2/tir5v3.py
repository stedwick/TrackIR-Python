from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import random
import time
from typing import Sequence

import usb.core
import usb.util

TIR_VENDOR_ID = 0x131D
TIR5V3_PRODUCT_ID = 0x0159
TIR5V3_FRAME_WIDTH = 640
TIR5V3_FRAME_HEIGHT = 480
TIR5V3_INTERFACE = 0
TIR5V3_READ_SIZE = 0x4000
TIR5V3_DEFAULT_THRESHOLD = 0x96
TIR5V3_DEFAULT_BRIGHTNESS_RAW = 0x780


@dataclass(frozen=True)
class TIR5V3Status:
    raw: bytes
    length: int
    message_id: int
    stage: int
    firmware_loaded: bool
    status_flag: int
    checksum_hi: int
    checksum_lo: int


@dataclass(frozen=True)
class TIR5V3Stripe:
    hstart: int
    hstop: int
    vline: int
    points: int
    sum_x: int
    sum: int


@dataclass(frozen=True)
class TIR5V3Packet:
    raw: bytes
    packet_no: int | None
    packet_type: int
    payload_size: int
    stripes: tuple[TIR5V3Stripe, ...]

    @property
    def is_empty(self) -> bool:
        return not self.stripes


def apply_tir5v3_transport(packet: Sequence[int], r1: int) -> bytes:
    if len(packet) != 24:
        raise ValueError("TIR5V3 transport packets must be 24 bytes")
    if not 0 <= r1 <= 0xFF:
        raise ValueError("r1 must fit in one byte")

    encoded = bytearray(packet)
    encoded[0] ^= 0x69 ^ encoded[r1 >> 4] ^ encoded[r1 & 0x0F]
    encoded[17] = r1
    return bytes(encoded)


def is_tir5v3_stream_header(data: bytes) -> bool:
    return (
        len(data) >= 4
        and data[1] == 0x10
        and data[2] in (0x00, 0x05)
        and (data[0] ^ data[1] ^ data[2] ^ data[3]) == 0xAA
    )


def is_tir5v3_type3_prefix(data: bytes) -> bool:
    return len(data) >= 3 and data[0] >= 3 and data[1] == 0x10 and data[2] == 0x03


def extract_tir5v3_packets(stream_buffer: bytes) -> tuple[list[bytes], bytes]:
    packets: list[bytes] = []
    pending = bytes(stream_buffer)

    while pending:
        start, packet, incomplete_start = _find_next_complete_packet(pending)
        if packet is None:
            if incomplete_start is not None:
                return packets, pending[incomplete_start:]
            return packets, pending[-7:]

        if start > 0:
            pending = pending[start:]

        packets.append(packet)
        pending = pending[len(packet) :]

    return packets, b""


def parse_tir5v3_status(packet: bytes) -> TIR5V3Status | None:
    if len(packet) < 7 or packet[1] != 0x20:
        return None

    return TIR5V3Status(
        raw=bytes(packet),
        length=packet[0],
        message_id=packet[1],
        stage=packet[2],
        firmware_loaded=packet[3] == 0x01,
        status_flag=packet[4],
        checksum_hi=packet[5],
        checksum_lo=packet[6],
    )


def decode_tir5v3_stripe(payload: bytes) -> TIR5V3Stripe:
    if len(payload) != 8:
        raise ValueError("TIR5V3 stripes are 8 bytes")

    hstart = (payload[0] << 2) | (payload[1] >> 6)
    vline = ((payload[1] & 0x3F) << 3) | ((payload[2] & 0xE0) >> 5)
    points = ((payload[2] & 0x1F) << 5) | (payload[3] >> 3)
    sum_x = ((payload[3] & 0x07) << 17) | (payload[4] << 9) | (payload[5] << 1) | (payload[6] >> 7)
    stripe_sum = ((payload[6] & 0x7F) << 8) | payload[7]
    hstop = hstart + points - 1

    return TIR5V3Stripe(
        hstart=hstart,
        hstop=hstop,
        vline=vline,
        points=points,
        sum_x=sum_x,
        sum=stripe_sum,
    )


def decode_tir4_style_stripe(payload: bytes) -> TIR5V3Stripe:
    if len(payload) != 4:
        raise ValueError("TIR4-style stripes are 4 bytes")

    vline = payload[0]
    hstart = payload[1]
    hstop = payload[2]
    rest = payload[3]
    if rest & 0x20:
        vline |= 0x100
    if rest & 0x80:
        hstart |= 0x100
    if rest & 0x40:
        hstop |= 0x100
    if rest & 0x10:
        hstart |= 0x200
    if rest & 0x08:
        hstop |= 0x200

    points = hstop - hstart + 1
    stripe_sum = points
    sum_x = points * (points - 1) // 2
    return TIR5V3Stripe(
        hstart=hstart,
        hstop=hstop,
        vline=vline,
        points=points,
        sum_x=sum_x,
        sum=stripe_sum,
    )


def parse_tir5v3_packet(packet: bytes) -> TIR5V3Packet:
    if len(packet) < 3:
        raise ValueError("Packet is too short")

    if is_tir5v3_stream_header(packet):
        packet_no = packet[0]
        packet_type = packet[2]
        payload_size = int.from_bytes(packet[-4:], "big")
        payload = packet[4:-4]
        if payload_size != len(payload):
            raise ValueError("Packet payload size does not match trailer")

        stripe_size = 8 if packet_type == 0x05 else 4
        decoder = decode_tir5v3_stripe if packet_type == 0x05 else decode_tir4_style_stripe
        stripes = []
        for offset in range(0, len(payload), stripe_size):
            chunk = payload[offset : offset + stripe_size]
            if len(chunk) < stripe_size:
                break
            stripes.append(decoder(chunk))

        return TIR5V3Packet(
            raw=bytes(packet),
            packet_no=packet_no,
            packet_type=packet_type,
            payload_size=payload_size,
            stripes=tuple(stripes),
        )

    if is_tir5v3_type3_prefix(packet):
        return TIR5V3Packet(
            raw=bytes(packet),
            packet_no=None,
            packet_type=0x03,
            payload_size=max(0, len(packet) - 2),
            stripes=(),
        )

    raise ValueError("Unsupported packet format")


def compute_weighted_centroid(stripes: Sequence[TIR5V3Stripe]) -> tuple[float, float] | None:
    if not stripes:
        return None

    total_weight = sum(stripe.sum for stripe in stripes if stripe.sum > 0)
    if total_weight <= 0:
        return None

    sum_x = sum((stripe.sum * stripe.hstart) + stripe.sum_x for stripe in stripes)
    sum_y = sum(stripe.sum * stripe.vline for stripe in stripes)
    return (sum_x / total_weight, sum_y / total_weight)


def default_log_path(mode: str) -> Path:
    timestamp = time.strftime("%Y%m%d-%H%M%S")
    return Path("tmp/logs") / f"tir5v3-{mode}-{timestamp}.log"


class TrackIRTIR5V3:
    def __init__(
        self,
        *,
        vendor_id: int = TIR_VENDOR_ID,
        product_id: int = TIR5V3_PRODUCT_ID,
        read_size: int = TIR5V3_READ_SIZE,
        timeout_ms: int = 200,
        rng: random.Random | None = None,
        logger=None,
    ) -> None:
        self.vendor_id = vendor_id
        self.product_id = product_id
        self.read_size = read_size
        self.timeout_ms = timeout_ms
        self.rng = rng or random.Random()
        self.logger = logger
        self.device = None
        self.ep_in = None
        self.ep_out = None

    def open(self) -> None:
        self.device = usb.core.find(idVendor=self.vendor_id, idProduct=self.product_id)
        if self.device is None:
            raise ValueError(f"TrackIR TIR5V3 device {self.vendor_id:04x}:{self.product_id:04x} not found")

        self.device.set_configuration()
        cfg = self.device.get_active_configuration()
        intf = cfg[(TIR5V3_INTERFACE, 0)]

        self.ep_out = usb.util.find_descriptor(
            intf,
            custom_match=lambda ep: usb.util.endpoint_direction(ep.bEndpointAddress) == usb.util.ENDPOINT_OUT,
        )
        self.ep_in = usb.util.find_descriptor(
            intf,
            custom_match=lambda ep: usb.util.endpoint_direction(ep.bEndpointAddress) == usb.util.ENDPOINT_IN,
        )

        if self.ep_out is None or self.ep_in is None:
            raise ValueError("Could not find TrackIR TIR5V3 bulk endpoints")

    def close(self) -> None:
        if self.device is not None:
            usb.util.dispose_resources(self.device)
            self.device = None
            self.ep_in = None
            self.ep_out = None

    def device_summary(self) -> dict[str, int]:
        if self.device is None or self.ep_in is None or self.ep_out is None:
            raise RuntimeError("Device is not open")

        cfg = self.device.get_active_configuration()
        intf = cfg[(TIR5V3_INTERFACE, 0)]
        return {
            "vendor_id": self.device.idVendor,
            "product_id": self.device.idProduct,
            "endpoint_out": self.ep_out.bEndpointAddress,
            "endpoint_in": self.ep_in.bEndpointAddress,
            "interface": intf.bInterfaceNumber,
            "max_packet_size": self.ep_in.wMaxPacketSize,
        }

    def initialize(self) -> list[TIR5V3Status]:
        statuses: list[TIR5V3Status] = []
        self._send_intent_2(0x1A, 0x00)
        self._send_intent_2(0x1A, 0x00)
        self._send_intent_1(0x13)

        for subcommand in (0x01, 0x02, 0x03):
            status = self.read_status()
            statuses.append(status)
            self._send_intent_2(0x1A, subcommand)

        statuses.append(self.read_status())
        self.set_threshold(TIR5V3_DEFAULT_THRESHOLD)
        self.set_ir_brightness_raw(TIR5V3_DEFAULT_BRIGHTNESS_RAW)
        self.set_ir_led(True)
        self._send_intent_4(0x19, 0x03, 0x00, 0x05)
        self._send_intent_4(0x19, 0x04, 0x00, 0x00)
        return statuses

    def start_streaming(self) -> None:
        self.set_ir_led(True)
        self.set_threshold(TIR5V3_DEFAULT_THRESHOLD)
        self.set_threshold(TIR5V3_DEFAULT_THRESHOLD)
        self._send_intent_4(0x19, 0x04, 0x00, 0x00)
        self._send_intent_4(0x19, 0x03, 0x00, 0x05)
        self._send_intent_2(0x1A, 0x04)
        self.set_ir_led(True)

    def stop_streaming(self) -> None:
        self._send_intent_2(0x1A, 0x05)
        self.set_ir_led(False)
        self._send_intent_2(0x1A, 0x06)
        self._send_intent_1(0x13)

    def read_status(self) -> TIR5V3Status:
        self._send_intent_2(0x1A, 0x07)
        deadline = time.time() + 2.0
        while time.time() < deadline:
            chunk = self.read_chunk(timeout_ms=100)
            if not chunk:
                continue
            status = parse_tir5v3_status(chunk)
            if status is not None:
                return status

        raise TimeoutError("Timed out waiting for a TIR5V3 status packet")

    def read_chunk(self, *, timeout_ms: int | None = None) -> bytes:
        if self.ep_in is None:
            raise RuntimeError("Device is not open")

        effective_timeout = self.timeout_ms if timeout_ms is None else timeout_ms
        try:
            data = self.ep_in.read(self.read_size, timeout=effective_timeout)
            chunk = bytes(data)
            if self.logger is not None:
                self.logger.log_recv(chunk)
            return chunk
        except usb.core.USBError as error:
            if getattr(error, "backend_error_code", None) == -7 or getattr(error, "errno", None) == 60:
                return b""
            raise

    def set_threshold(self, threshold: int) -> None:
        self._send_intent_4(0x19, 0x05, threshold >> 7, (threshold << 1) & 0xFF)

    def set_ir_led(self, enabled: bool) -> None:
        self._send_intent_4(0x19, 0x09, 0x00, 0x01 if enabled else 0x00)

    def set_ir_brightness_raw(self, brightness: int) -> None:
        self._send_intent_4(0x23, 0x35, 0x02, brightness & 0xFF)
        self._send_intent_4(0x23, 0x35, 0x01, brightness >> 8)
        self._send_intent_4(0x23, 0x35, 0x00, 0x00)
        self._send_intent_4(0x23, 0x3B, 0x8F, (brightness >> 4) & 0xFF)
        self._send_intent_4(0x23, 0x3B, 0x8E, brightness >> 12)

    def _send_intent_1(self, value_1: int, *, wait_us: int = 50_000) -> None:
        packet = [value_1] + [self.rng.randrange(256) for _ in range(23)]
        self._send_packet(packet, wait_us=wait_us, label=f"{value_1:02x}")

    def _send_intent_2(self, value_1: int, value_2: int, *, wait_us: int = 50_000) -> None:
        packet = [value_1] + [self.rng.randrange(256) for _ in range(23)]
        r1 = self._rand_range(2, 14)
        r2 = self._rand_range(0, 3)
        packet[1] ^= (packet[1] ^ r1) & 0x08
        packet[2] ^= (packet[2] ^ r1) & 0x04
        packet[3] ^= (packet[3] ^ r1) & 0x02
        packet[4] ^= (packet[4] ^ r1) & 0x01
        packet[r1] = ((value_2 << 4) | (packet[r1] & 0x0F)) ^ (packet[16] & 0xF0)
        packet[r1 + 1] = (r2 << 6) | (packet[r1 + 1] & 0x3F)
        self._send_packet(packet, wait_us=wait_us, label=f"{value_1:02x}{value_2:02x}")

    def _send_intent_4(self, value_1: int, value_2: int, value_3: int, value_4: int, *, wait_us: int = 50_000) -> None:
        packet = [value_1] + [self.rng.randrange(256) for _ in range(23)]
        r1 = self._rand_range(6, 14)
        packet[1] ^= (packet[1] ^ r1) & 0x08
        packet[2] ^= (packet[2] ^ r1) & 0x04
        packet[3] ^= (packet[3] ^ r1) & 0x02
        packet[4] ^= (packet[4] ^ r1) & 0x01
        packet[r1] = packet[16] ^ value_2
        packet[r1 - 1] = packet[19] ^ value_3
        packet[r1 + 1] = packet[18] ^ value_4
        self._send_packet(packet, wait_us=wait_us, label=f"{value_1:02x}{value_2:02x}{value_3:02x}{value_4:02x}")

    def _send_packet(self, packet: Sequence[int], *, wait_us: int, label: str) -> None:
        if self.ep_out is None:
            raise RuntimeError("Device is not open")

        r1 = (self._rand_range(1, 15) << 4) + self._rand_range(1, 15)
        encoded = apply_tir5v3_transport(packet, r1)
        if self.logger is not None:
            self.logger.log_send(label, encoded)
        self.ep_out.write(encoded, timeout=1000)
        if wait_us > 0:
            time.sleep(wait_us / 1_000_000)

    def _rand_range(self, low: int, high: int) -> int:
        if high <= low:
            return high
        return low + self.rng.randrange(high - low + 1)


def _find_next_packet_start(data: bytes) -> int:
    for index in range(max(0, len(data) - 2)):
        window = data[index:]
        if is_tir5v3_stream_header(window) or is_tir5v3_type3_prefix(window):
            return index
    return -1


def _find_next_complete_packet(data: bytes) -> tuple[int, bytes | None, int | None]:
    incomplete_start = None

    for index in range(max(0, len(data) - 2)):
        window = data[index:]
        if is_tir5v3_stream_header(window):
            packet = _extract_type05_packet(window)
            if packet is not None:
                return index, packet, None
            if incomplete_start is None:
                incomplete_start = index
            continue

        if is_tir5v3_type3_prefix(window):
            packet_length = window[0]
            if len(window) >= packet_length:
                return index, bytes(window[:packet_length]), None
            if incomplete_start is None:
                incomplete_start = index

    return -1, None, incomplete_start


def _extract_type05_packet(data: bytes) -> bytes | None:
    if len(data) < 8:
        return None

    for packet_length in range(8, len(data) + 1, 8):
        payload_size = int.from_bytes(data[packet_length - 4 : packet_length], "big")
        if payload_size != packet_length - 8:
            continue
        if packet_length < len(data) and not _looks_like_packet_prefix(data[packet_length:]):
            continue
        return bytes(data[:packet_length])

    return None


def _looks_like_packet_start(data: bytes) -> bool:
    return is_tir5v3_stream_header(data) or is_tir5v3_type3_prefix(data)


def _looks_like_packet_prefix(data: bytes) -> bool:
    if not data:
        return True
    if _looks_like_packet_start(data):
        return True
    if len(data) == 1:
        return True
    if len(data) == 2:
        return data[1] == 0x10
    if len(data) == 3:
        return data[1] == 0x10 and data[2] in (0x00, 0x03, 0x05)
    return False
