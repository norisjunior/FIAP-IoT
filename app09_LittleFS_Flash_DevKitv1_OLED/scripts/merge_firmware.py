Import("env")

from os.path import join
from subprocess import run


def merge_firmware(source, target, env):
    build_dir = env.subst("$BUILD_DIR")
    platform = env.PioPlatform()
    esptool = join(platform.get_package_dir("tool-esptoolpy"), "esptool.py")
    framework_dir = platform.get_package_dir("framework-arduinoespressif32")

    command = [
        env.subst("$PYTHONEXE"),
        esptool,
        "--chip",
        "esp32",
        "merge_bin",
        "--output",
        join(build_dir, "firmware-merged.bin"),
        "0x1000",
        join(build_dir, "bootloader.bin"),
        "0x8000",
        join(build_dir, "partitions.bin"),
        "0xE000",
        join(framework_dir, "tools", "partitions", "boot_app0.bin"),
        "0x10000",
        join(build_dir, "firmware.bin"),
    ]

    print("Generating complete Wokwi flash image")
    run(command, check=True)


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", merge_firmware)
