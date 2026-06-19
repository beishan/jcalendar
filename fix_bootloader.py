"""Post-build script for ESP32-S3-N16R8 bootloader.
Patches flash mode byte (offset 2) from DIO to QIO in bootloader.bin.
N16R8 flash requires QIO mode; PlatformIO's BOARD_FLASH_MODE may remain "dio"
despite board_build.flash_mode = qio.
"""
Import("env")
import os

def fix_bootloader(source, target, env):
    for t in target:
        path = str(t)
        if not os.path.exists(path) or not path.endswith("bootloader.bin"):
            continue
        with open(path, 'r+b') as f:
            f.seek(2)
            current = f.read(1)[0]
            if current != 0x00:
                mode_names = {0x00: "QIO", 0x02: "DIO", 0x03: "DOUT"}
                f.seek(2)
                f.write(b'\x00')
                print(f"fix_bootloader: flash mode {mode_names.get(current, hex(current))} -> QIO")

build_dir = env.subst("$BUILD_DIR")
bootloader_target = os.path.join(build_dir, "bootloader.bin")
if env.subst("$PIOENV") == "esp32s3":
    env.AddPostAction(bootloader_target, fix_bootloader)
