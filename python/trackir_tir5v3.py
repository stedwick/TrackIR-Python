from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import sys
import time

import cv2
import numpy as np

from tir5v3 import (
    TIR5V3_FRAME_HEIGHT,
    TIR5V3_FRAME_WIDTH,
    TIR5V3Packet,
    TrackIRTIR5V3,
    compute_weighted_centroid,
    default_log_path,
    extract_tir5v3_packets,
    parse_tir5v3_packet,
)

WINDOW_NAME = "TrackIR TIR5V3 Preview"


@dataclass(frozen=True)
class FrameStats:
    frame_index: int
    packet_type: int
    stripe_count: int
    centroid: tuple[float, float] | None
    packet_no: int | None


class SessionLogger:
    def __init__(self, path: Path) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        self.path = path
        self._handle = path.open("a", encoding="utf-8")

    def close(self) -> None:
        self._handle.close()

    def log_event(self, message: str) -> None:
        self._handle.write(f"{_timestamp()} EVENT {message}\n")
        self._handle.flush()

    def log_send(self, label: str, data: bytes) -> None:
        payload = data.hex(" ")
        self._handle.write(f"{_timestamp()} SEND {label} {payload}\n")
        self._handle.flush()

    def log_recv(self, data: bytes) -> None:
        payload = data.hex(" ")
        self._handle.write(f"{_timestamp()} RECV {len(data)} {payload}\n")
        self._handle.flush()


def empty_frame_stats(packet_no: int | None = None) -> FrameStats:
    return FrameStats(
        frame_index=0,
        packet_type=0x05,
        stripe_count=0,
        centroid=None,
        packet_no=packet_no,
    )


def build_preview_frame(packet: TIR5V3Packet, stats: FrameStats, scale: int) -> np.ndarray:
    frame = np.zeros((TIR5V3_FRAME_HEIGHT, TIR5V3_FRAME_WIDTH), dtype=np.uint8)

    for stripe in packet.stripes:
        if stripe.points <= 0 or not 0 <= stripe.vline < TIR5V3_FRAME_HEIGHT:
            continue
        x1 = max(0, min(TIR5V3_FRAME_WIDTH - 1, stripe.hstart))
        x2 = max(0, min(TIR5V3_FRAME_WIDTH - 1, stripe.hstop))
        brightness = max(32, min(255, round(stripe.sum / max(1, stripe.points))))
        frame[stripe.vline, x1 : x2 + 1] = brightness

    image = cv2.cvtColor(frame, cv2.COLOR_GRAY2BGR)
    if stats.centroid is not None:
        cx = max(0, min(TIR5V3_FRAME_WIDTH - 1, int(round(stats.centroid[0]))))
        cy = max(0, min(TIR5V3_FRAME_HEIGHT - 1, int(round(stats.centroid[1]))))
        cv2.drawMarker(
            image,
            (cx, cy),
            (0, 255, 0),
            markerType=cv2.MARKER_CROSS,
            markerSize=12,
            thickness=1,
        )

    overlay = [
        f"frame={stats.frame_index}",
        f"type=0x{stats.packet_type:02x}",
        f"packet={stats.packet_no if stats.packet_no is not None else '-'}",
        f"stripes={stats.stripe_count}",
    ]
    if stats.centroid is not None:
        overlay.append(f"x={stats.centroid[0]:.1f} y={stats.centroid[1]:.1f}")
    else:
        overlay.append("x=- y=-")
    cv2.putText(
        image,
        "  ".join(overlay),
        (10, 22),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.55,
        (255, 255, 255),
        1,
        cv2.LINE_AA,
    )

    if scale > 1:
        image = cv2.resize(
            image,
            (TIR5V3_FRAME_WIDTH * scale, TIR5V3_FRAME_HEIGHT * scale),
            interpolation=cv2.INTER_NEAREST,
        )
    return image


def packet_stats(packet: TIR5V3Packet, frame_index: int) -> FrameStats:
    return FrameStats(
        frame_index=frame_index,
        packet_type=packet.packet_type,
        stripe_count=len(packet.stripes),
        centroid=compute_weighted_centroid(packet.stripes),
        packet_no=packet.packet_no,
    )


def should_print_coordinate_sample(
    elapsed_since_last_sample: float | None,
    interval_seconds: float = 1.0,
) -> bool:
    return elapsed_since_last_sample is None or elapsed_since_last_sample >= interval_seconds


def print_statuses(statuses) -> None:
    for index, status in enumerate(statuses, start=1):
        print(
            f"status[{index}] stage=0x{status.stage:02x} "
            f"firmware_loaded={int(status.firmware_loaded)} "
            f"flag=0x{status.status_flag:02x}"
        )


def preview_packets(
    trackir: TrackIRTIR5V3,
    logger: SessionLogger,
    scale: int,
    seconds: float | None,
    show_opencv_preview: bool,
) -> None:
    pending = b""
    frame_index = 0
    deadline = time.monotonic() + seconds if seconds is not None else None
    last_coordinate_sample_time: float | None = None
    empty_packet = TIR5V3Packet(raw=b"", packet_no=None, packet_type=0x05, payload_size=0, stripes=())

    if show_opencv_preview:
        cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)
        print("Controls: q to quit")
        cv2.imshow(WINDOW_NAME, build_preview_frame(empty_packet, empty_frame_stats(), scale))
    else:
        print("Preview disabled. Printing x/y once per second. Press Ctrl+C to quit.")

    logger.log_event("preview_started")

    while True:
        now = time.monotonic()
        if deadline is not None and now >= deadline:
            break

        chunk = trackir.read_chunk(timeout_ms=50)
        if chunk:
            pending += chunk
            packets, pending = extract_tir5v3_packets(pending)
            for raw_packet in packets:
                packet = parse_tir5v3_packet(raw_packet)
                if packet.packet_type not in (0x00, 0x05):
                    continue

                frame_index += 1
                stats = packet_stats(packet, frame_index)

                if show_opencv_preview:
                    image = build_preview_frame(packet, stats, scale)
                    cv2.imshow(WINDOW_NAME, image)
                elif should_print_coordinate_sample(
                    None if last_coordinate_sample_time is None else now - last_coordinate_sample_time
                ):
                    if stats.centroid is None:
                        print(f"frame={stats.frame_index} x=- y=-")
                    else:
                        print(
                            f"frame={stats.frame_index} "
                            f"x={int(stats.centroid[0])} y={int(stats.centroid[1])}"
                        )
                    last_coordinate_sample_time = now

                if not packet.is_empty:
                    logger.log_event(
                        f"frame={frame_index} packet={stats.packet_no} "
                        f"type=0x{stats.packet_type:02x} stripes={stats.stripe_count} "
                        f"centroid={stats.centroid}"
                    )

        if show_opencv_preview:
            key = cv2.waitKey(1) & 0xFF
            if key == ord("q"):
                break

    if show_opencv_preview:
        cv2.destroyAllWindows()


def dump_packets(trackir: TrackIRTIR5V3, logger: SessionLogger, seconds: float, include_empty: bool) -> None:
    pending = b""
    frame_index = 0
    deadline = time.monotonic() + seconds

    while time.monotonic() < deadline:
        chunk = trackir.read_chunk(timeout_ms=100)
        if not chunk:
            continue

        pending += chunk
        packets, pending = extract_tir5v3_packets(pending)
        for raw_packet in packets:
            packet = parse_tir5v3_packet(raw_packet)
            if packet.packet_type not in (0x00, 0x05):
                continue
            if packet.is_empty and not include_empty:
                continue

            frame_index += 1
            stats = packet_stats(packet, frame_index)
            print(
                f"frame={stats.frame_index} packet={stats.packet_no} "
                f"type=0x{stats.packet_type:02x} stripes={stats.stripe_count} "
                f"centroid={stats.centroid}"
            )
            logger.log_event(
                f"dump frame={stats.frame_index} packet={stats.packet_no} "
                f"type=0x{stats.packet_type:02x} stripes={stats.stripe_count} "
                f"centroid={stats.centroid}"
            )


def run_identify(args: argparse.Namespace) -> None:
    logger = SessionLogger(_resolve_log_path(args.log, "identify"))
    trackir = TrackIRTIR5V3(logger=logger)

    try:
        trackir.open()
        summary = trackir.device_summary()
        print(
            "device "
            f"{summary['vendor_id']:04x}:{summary['product_id']:04x} "
            f"in=0x{summary['endpoint_in']:02x} out=0x{summary['endpoint_out']:02x} "
            f"max_packet={summary['max_packet_size']}"
        )
        statuses = trackir.initialize()
        print_statuses(statuses)
        print(f"log={logger.path}")
    finally:
        trackir.close()
        logger.close()


def run_streaming_session(args: argparse.Namespace) -> None:
    logger = SessionLogger(_resolve_log_path(args.log, args.mode))
    trackir = TrackIRTIR5V3(logger=logger)
    started = False

    try:
        trackir.open()
        summary = trackir.device_summary()
        print(
            "device "
            f"{summary['vendor_id']:04x}:{summary['product_id']:04x} "
            f"in=0x{summary['endpoint_in']:02x} out=0x{summary['endpoint_out']:02x}"
        )
        statuses = trackir.initialize()
        print_statuses(statuses)
        trackir.start_streaming()
        started = True
        print(f"log={logger.path}")

        if args.mode == "opencv":
            preview_packets(
                trackir,
                logger,
                args.scale,
                args.seconds,
                True,
            )
        elif args.mode == "log":
            preview_packets(trackir, logger, scale=1, seconds=args.seconds, show_opencv_preview=False)
        else:
            dump_packets(trackir, logger, args.seconds, args.include_empty)
    finally:
        if started:
            trackir.stop_streaming()
        trackir.close()
        logger.close()


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="TrackIR TIR5V3 probe, preview, and logging tool")
    subparsers = parser.add_subparsers(dest="mode", required=True)

    identify = subparsers.add_parser("identify", help="Open the device and print init/status info")
    identify.add_argument("--log", type=Path, default=None, help="Write session log to this file")

    dump = subparsers.add_parser("dump", help="Print parsed packets for a fixed duration")
    dump.add_argument("--seconds", type=float, default=5.0, help="How long to read packets")
    dump.add_argument("--include-empty", action="store_true", help="Show empty packets too")
    dump.add_argument("--log", type=Path, default=None, help="Write session log to this file")

    opencv = subparsers.add_parser("opencv", help="Show parsed stripes and centroid in an OpenCV window")
    opencv.add_argument("--seconds", type=float, default=None, help="Optional auto-exit timeout")
    opencv.add_argument("--scale", type=int, default=2, help="Nearest-neighbor display scale")
    opencv.add_argument("--log", type=Path, default=None, help="Write session log to this file")

    log = subparsers.add_parser("log", help="Print centroid x/y once per second without OpenCV")
    log.add_argument("--seconds", type=float, default=None, help="Optional auto-exit timeout")
    log.add_argument("--log", type=Path, default=None, help="Write session log to this file")

    return parser


def main() -> None:
    parser = build_parser()
    argv = sys.argv[1:]
    if not argv or argv[0].startswith("-"):
        argv = ["opencv", *argv]
    args = parser.parse_args(argv)

    if args.mode == "identify":
        run_identify(args)
        return

    run_streaming_session(args)


def _resolve_log_path(log_path: Path | None, mode: str) -> Path:
    if log_path is not None:
        return log_path
    return default_log_path(mode)


def _timestamp() -> str:
    return time.strftime("%Y-%m-%d %H:%M:%S")


if __name__ == "__main__":
    main()
