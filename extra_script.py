Import("env")
import os

def merge_bin(source, target, env):
    firmware_path = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")
    bootloader_path = os.path.join(env.subst("$BUILD_DIR"), "bootloader.bin")
    partitions_path = os.path.join(env.subst("$BUILD_DIR"), "partitions.bin")
    filesystem_path = os.path.join(env.subst("$BUILD_DIR"), "littlefs.bin")
    output_path = os.path.join(env.subst("$PROJECT_DIR"), "dist", "firmware.bin")
    board_mcu = env.BoardConfig().get("build.mcu", "esp32")
    chip = "esp32s3" if board_mcu == "esp32s3" else "esp32"
    bootloader_offset = "0x0" if chip == "esp32s3" else "0x1000"
    filesystem_offset = "0x810000" if chip == "esp32s3" else "0x3D0000"
    
    cmd = [
        "esptool.py",
        "--chip", chip,
        "merge_bin",
        "-o", output_path,
        bootloader_offset, bootloader_path,
        "0x8000", partitions_path,
        "0x10000", firmware_path,
        filesystem_offset, filesystem_path
    ]
    env.Execute(" ".join(cmd))

env.AddPostAction("buildprog", merge_bin)
