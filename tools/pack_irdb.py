#!/usr/bin/env python3
"""Pack the pinned Flipper Devices TV IRDB tree into one seekable asset."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

MAGIC = b"TVRDB1\0\0"
HEADER = struct.Struct("<8s6I")
BRAND = struct.Struct("<52s3I")
MODEL = struct.Struct("<64s4I")


def fixed(text: str, size: int) -> bytes:
    raw = text.encode("utf-8")
    if len(raw) >= size:
        raw = raw[: size - 1]
        while raw and (raw[-1] & 0xC0) == 0x80:
            raw = raw[:-1]
    return raw + bytes(size - len(raw))


def display_name(path: Path) -> str:
    return path.stem.replace("_", " ")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    groups: list[tuple[str, list[Path]]] = []
    for brand_dir in sorted((p for p in args.source.iterdir() if p.is_dir()), key=lambda p: p.name.casefold()):
        files = sorted(brand_dir.rglob("*.ir"), key=lambda p: str(p).casefold())
        if files:
            groups.append((brand_dir.name.replace("_", " "), files))

    brand_count = len(groups)
    model_count = sum(len(files) for _, files in groups)
    brand_offset = HEADER.size
    model_offset = brand_offset + brand_count * BRAND.size
    payload_offset = model_offset + model_count * MODEL.size

    brands: list[bytes] = []
    models: list[bytes] = []
    payloads: list[bytes] = []
    model_index = 0
    payload_cursor = payload_offset
    for brand_name, files in groups:
        brands.append(BRAND.pack(fixed(brand_name, 52), model_index, len(files), 0))
        for file_path in files:
            data = file_path.read_bytes()
            models.append(
                MODEL.pack(fixed(display_name(file_path), 64), payload_cursor, len(data), model_index, 0)
            )
            payloads.append(data)
            payload_cursor += len(data)
            model_index += 1

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("wb") as output:
        output.write(
            HEADER.pack(MAGIC, 1, brand_count, model_count, brand_offset, model_offset, payload_offset)
        )
        output.writelines(brands)
        output.writelines(models)
        output.writelines(payloads)

    print(f"packed {brand_count} brands, {model_count} remotes, {payload_cursor} bytes")


if __name__ == "__main__":
    main()
