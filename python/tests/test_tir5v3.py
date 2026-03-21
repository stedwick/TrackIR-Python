from __future__ import annotations

import unittest

from tir5v3 import (
    TIR5V3Stripe,
    apply_tir5v3_transport,
    compute_weighted_centroid,
    decode_tir5v3_stripe,
    extract_tir5v3_packets,
    parse_tir5v3_packet,
    shutdown_steps,
)
from trackir_tir5v3 import empty_frame_stats, should_print_coordinate_sample


def make_type5_packet(packet_no: int, payload: bytes = b"") -> bytes:
    header = bytes([packet_no, 0x10, 0x05, packet_no ^ 0x10 ^ 0x05 ^ 0xAA])
    trailer = len(payload).to_bytes(4, "big")
    return header + payload + trailer


def encode_tir5v3_stripe(
    *,
    hstart: int,
    vline: int,
    points: int,
    sum_x: int,
    stripe_sum: int,
) -> bytes:
    return bytes(
        [
            (hstart >> 2) & 0xFF,
            ((hstart & 0x03) << 6) | ((vline >> 3) & 0x3F),
            ((vline & 0x07) << 5) | ((points >> 5) & 0x1F),
            ((points & 0x1F) << 3) | ((sum_x >> 17) & 0x07),
            (sum_x >> 9) & 0xFF,
            (sum_x >> 1) & 0xFF,
            ((sum_x & 0x01) << 7) | ((stripe_sum >> 8) & 0x7F),
            stripe_sum & 0xFF,
        ]
    )


class TIR5V3Tests(unittest.TestCase):
    def test_empty_frame_stats_marks_waiting_preview_state(self) -> None:
        stats = empty_frame_stats(packet_no=0x77)

        self.assertEqual(stats.frame_index, 0)
        self.assertEqual(stats.packet_type, 0x05)
        self.assertEqual(stats.stripe_count, 0)
        self.assertIsNone(stats.centroid)
        self.assertEqual(stats.packet_no, 0x77)

    def test_should_print_coordinate_sample_uses_one_second_interval(self) -> None:
        self.assertTrue(should_print_coordinate_sample(None))
        self.assertFalse(should_print_coordinate_sample(0.5))
        self.assertTrue(should_print_coordinate_sample(1.0))
        self.assertTrue(should_print_coordinate_sample(1.5))

    def test_apply_tir5v3_transport_obfuscates_header_and_nonce(self) -> None:
        packet = bytes(range(24))
        encoded = apply_tir5v3_transport(packet, 0xAB)

        expected_first = packet[0] ^ 0x69 ^ packet[0x0A] ^ packet[0x0B]
        self.assertEqual(encoded[0], expected_first)
        self.assertEqual(encoded[17], 0xAB)
        self.assertEqual(encoded[1:17], packet[1:17])
        self.assertEqual(encoded[18:], packet[18:])

    def test_decode_tir5v3_stripe_decodes_all_fields(self) -> None:
        payload = encode_tir5v3_stripe(
            hstart=321,
            vline=245,
            points=6,
            sum_x=15,
            stripe_sum=700,
        )

        stripe = decode_tir5v3_stripe(payload)

        self.assertEqual(stripe.hstart, 321)
        self.assertEqual(stripe.hstop, 326)
        self.assertEqual(stripe.vline, 245)
        self.assertEqual(stripe.points, 6)
        self.assertEqual(stripe.sum_x, 15)
        self.assertEqual(stripe.sum, 700)

    def test_extract_tir5v3_packets_recovers_split_stream_packets(self) -> None:
        packet_one = make_type5_packet(0x11)
        packet_two = make_type5_packet(0x12)

        packets, pending = extract_tir5v3_packets(b"\xff\x00" + packet_one + packet_two[:3])
        self.assertEqual(packets, [packet_one])
        self.assertEqual(pending, packet_two[:3])

        packets, pending = extract_tir5v3_packets(pending + packet_two[3:])
        self.assertEqual(packets, [packet_two])
        self.assertEqual(pending, b"")

    def test_extract_tir5v3_packets_resyncs_after_bad_leading_header(self) -> None:
        corrupted = bytes([0x11, 0x10, 0x05, 0xAE, 0x40, 0x4D, 0xE0, 0x58])
        packet = make_type5_packet(0x12)

        packets, pending = extract_tir5v3_packets(corrupted + packet)

        self.assertEqual(packets, [packet])
        self.assertEqual(pending, b"")

    def test_parse_tir5v3_packet_handles_empty_type5_packet(self) -> None:
        packet = parse_tir5v3_packet(make_type5_packet(0x24))

        self.assertEqual(packet.packet_no, 0x24)
        self.assertEqual(packet.packet_type, 0x05)
        self.assertEqual(packet.payload_size, 0)
        self.assertEqual(packet.stripes, ())
        self.assertTrue(packet.is_empty)

    def test_compute_weighted_centroid_matches_linuxtrack_formula(self) -> None:
        stripes = (
            TIR5V3Stripe(hstart=10, hstop=12, vline=4, points=3, sum_x=3, sum=3),
            TIR5V3Stripe(hstart=20, hstop=21, vline=6, points=2, sum_x=1, sum=2),
        )

        centroid = compute_weighted_centroid(stripes)

        self.assertIsNotNone(centroid)
        self.assertAlmostEqual(centroid[0], 14.8)
        self.assertAlmostEqual(centroid[1], 4.8)

    def test_shutdown_steps_turns_led_off_even_without_streaming(self) -> None:
        steps = shutdown_steps(is_streaming=False, ir_led_enabled=True)

        self.assertEqual(steps, (("intent_4", (0x19, 0x09, 0x00, 0x00)),))


if __name__ == "__main__":
    unittest.main()
