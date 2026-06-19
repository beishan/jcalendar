"""Post-build script to strip the 16-byte extended header added by esptool v4.11+.
ESP32-S3 v0.2 ROM bootloader only supports the standard 8-byte image header.
"""
Import("env")

import os

def strip_extended_header(source, target, env):
    """Remove 16-byte extended header (bytes 8-23) from firmware.bin and bootloader.bin."""
    for bin_file in target:
        path = str(bin_file)
        if not os.path.exists(path):
            continue
        with open(path, 'rb') as f:
            data = f.read()
        # Check if extended header exists (bytes 8-11 should be 0x000000ee for extended format)
        if len(data) > 24 and data[8:12] == b'\xee\x00\x00\x00':
            # Strip 16 bytes at offset 8
            stripped = data[:8] + data[24:]
            with open(path, 'wb') as f:
                f.write(stripped)
            print(f"Stripped extended header from {os.path.basename(path)}: {len(data)} -> {len(stripped)} bytes")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", strip_extended_header)
if env.subst("$PIOENV") != "esp32s3":
    env.AddPostAction("$BUILD_DIR/bootloader.bin", strip_extended_header)
