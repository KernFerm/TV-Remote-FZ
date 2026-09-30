#!/usr/bin/env python3
"""Pack verified TV IRDB trees into one seekable asset.

The primary tree is the official Flipper Devices IRDB.  An optional community
tree may add profiles that are not present in the official catalog.  Exact
brand/profile duplicates keep the official entry.
"""

from __future__ import annotations

import argparse
import re
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

MAGIC = b"TVRDB1\0\0"
HEADER = struct.Struct("<8s6I")
BRAND = struct.Struct("<52s3I")
MODEL = struct.Struct("<64s4I")

SOURCE_OFFICIAL = 0
SOURCE_COMMUNITY = 1


@dataclass(frozen=True)
class Profile:
    path: Path
    source_id: int


def fixed(text: str, size: int) -> bytes:
    raw = text.encode("utf-8")
    if len(raw) >= size:
        raw = raw[: size - 1]
        while raw and (raw[-1] & 0xC0) == 0x80:
            raw = raw[:-1]
    return raw + bytes(size - len(raw))


def display_name(path: Path) -> str:
    return path.stem.replace("_", " ")


def identity(text: str) -> str:
    """Normalize names only for duplicate detection, never for display."""
    return re.sub(r"[^a-z0-9]", "", text.casefold())


def is_flipper_ir_file(path: Path) -> bool:
    """Accept only files with the required Flipper IR header and version."""
    try:
        with path.open("rb") as file:
            first = file.readline().rstrip(b"\r\n")
            second = file.readline().rstrip(b"\r\n")
    except OSError as error:
        print(f"warning: skipping unreadable profile {path}: {error}", file=sys.stderr)
        return False
    if first == b"Filetype: IR signals file" and second == b"Version: 1":
        return True
    print(f"warning: skipping malformed Flipper IR profile: {path}", file=sys.stderr)
    return False


def add_tree(
    groups: dict[str, tuple[str, list[Profile], set[str]]], source: Path, source_id: int
) -> None:
    for brand_dir in sorted(
        (path for path in source.iterdir() if path.is_dir()), key=lambda path: path.name.casefold()
    ):
        brand_name = brand_dir.name.replace("_", " ")
        brand_key = identity(brand_name)
        if brand_key not in groups:
            groups[brand_key] = (brand_name, [], set())
        _, profiles, seen = groups[brand_key]
        for file_path in sorted(brand_dir.rglob("*.ir"), key=lambda path: str(path).casefold()):
            if not is_flipper_ir_file(file_path):
                continue
            profile_key = identity(display_name(file_path))
            if profile_key in seen:
                continue
            seen.add(profile_key)
            profiles.append(Profile(file_path, source_id))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument(
        "--community-source",
        type=Path,
        help="optional maintained Flipper-IRDB TVs directory; unique profiles are appended",
    )
    args = parser.parse_args()

    merged: dict[str, tuple[str, list[Profile], set[str]]] = {}
    add_tree(merged, args.source, SOURCE_OFFICIAL)
    if args.community_source:
        add_tree(merged, args.community_source, SOURCE_COMMUNITY)
    groups = sorted(
        ((brand_name, profiles) for brand_name, profiles, _ in merged.values() if profiles),
        key=lambda group: group[0].casefold(),
    )

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
    for brand_name, profiles in groups:
        brands.append(BRAND.pack(fixed(brand_name, 52), model_index, len(profiles), 0))
        for profile in profiles:
            data = profile.path.read_bytes()
            models.append(
                MODEL.pack(
                    fixed(display_name(profile.path), 64),
                    payload_cursor,
                    len(data),
                    model_index,
                    profile.source_id,
                )
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

    print(f"packed {brand_count} brands, {model_count} profiles, {payload_cursor} bytes")


if __name__ == "__main__":
    main()
