Import("env")
import os
import subprocess

# Force QIO flash mode for ESP32-S3-N16R8 at the SCons environment level
if env.subst("$PIOENV") == "esp32s3":
    env['BOARD_FLASH_MODE'] = 'qio'

def regenerate_bootloader_qio(env):
    """Regenerate bootloader.bin from OPI PSRAM ELF with QIO flash mode.
    N16R8 flash requires QIO; PlatformIO's board default is DIO.
    The bootloader_opi_80m.elf has OPI PSRAM support baked in;
    passing --flash_mode qio to elf2image sets QIO in the bootloader binary."""
    build_dir = env.subst("$BUILD_DIR")
    bootloader_bin = os.path.join(build_dir, "bootloader.bin")
    if not os.path.exists(bootloader_bin):
        return

    python_exe = env.subst("$PYTHONEXE")
    esptool_py = os.path.join(
        os.path.expanduser("~"),
        ".platformio", "packages",
        "tool-esptoolpy", "esptool.py"
    )
    bootloader_elf = os.path.join(
        os.path.expanduser("~"),
        ".platformio", "packages",
        "framework-arduinoespressif32",
        "tools", "sdk", "esp32s3", "bin",
        "bootloader_opi_80m.elf"
    )
    flash_size = env.BoardConfig().get("upload.flash_size", "16MB")

    if not os.path.exists(bootloader_elf):
        print("extra_script: bootloader OPI ELF not found, skipping QIO fix")
        return

    cmd = [
        python_exe, esptool_py,
        "--chip", "esp32s3",
        "elf2image",
        "--flash_mode", "qio",
        "--flash_freq", "80m",
        "--flash_size", flash_size,
        "-o", bootloader_bin,
        bootloader_elf
    ]
    print("extra_script: regenerating bootloader.bin with QIO flash mode...")
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print(f"extra_script: WARNING - bootloader QIO fix failed: {r.stderr[-200:]}")
    else:
        print("extra_script: bootloader.bin QIO fix applied")

def merge_bin(source, target, env):
    firmware_path = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")
    bootloader_path = os.path.join(env.subst("$BUILD_DIR"), "bootloader.bin")
    partitions_path = os.path.join(env.subst("$BUILD_DIR"), "partitions.bin")
    filesystem_path = os.path.join(env.subst("$BUILD_DIR"), "spiffs.bin")
    dist_dir = os.path.join(env.subst("$PROJECT_DIR"), "dist")
    if not os.path.exists(dist_dir):
        os.makedirs(dist_dir)
    output_path = os.path.join(dist_dir, "firmware.bin")

    pioenv = env.subst("$PIOENV")
    chip = "esp32s3" if "s3" in pioenv else "esp32"

    # For S3: regenerate bootloader with QIO flash mode before merging
    if chip == "esp32s3":
        regenerate_bootloader_qio(env)

    # SPIFFS offset depends on partition table:
    #   default_16MB.csv (S3):      spiffs at 0xc90000
    #   min_spiffs.csv  (non-S3):   spiffs at 0x3D0000
    fs_offset = "0xc90000" if "s3" in pioenv else "0x3D0000"

    cmd = [
        "esptool.py",
        "--chip", chip,
        "merge_bin",
        "-o", output_path,
        "0x1000", bootloader_path,
        "0x8000", partitions_path,
        "0x10000", firmware_path,
    ]
    if os.path.exists(filesystem_path):
        cmd.append(fs_offset)
        cmd.append(filesystem_path)

    env.Execute(" ".join(cmd))

env.AddPostAction("buildprog", merge_bin)
