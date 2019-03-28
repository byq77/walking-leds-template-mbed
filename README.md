# Walking Leds board (nRF5232) mbed template 

This template uses Mbed Cordio Link Layer instead Nordic SoftDevice.

## Web resources

* https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug
* https://marcelball.ca/projects/cortex-debug/
* https://marcelball.ca/projects/cortex-debug/cortex-debug-launch-configurations/
* https://github.com/ARMmbed/mbed-os
* https://github.com/ARMmbed/mbed-os-example-ble
* https://os.mbed.com/docs/mbed-cordio/19.02/introduction/index.html
* https://os.mbed.com/docs/mbed-os/v5.11/apis/bluetooth.html

## Prerequisites
* [Visual Studio Code IDE](https://code.visualstudio.com/)
* [mbed-cli tools](https://os.mbed.com/docs/v5.11/tools/installation-and-setup.html)
* [GNU Arm Embedded version 6 toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
* [nRF5x Command Line Tools](https://www.nordicsemi.com/DocLib/Content/User_Guides/nrf5x_cltools/latest/UG/cltools/nrf5x_command_line_tools_lpage)

## Setup

1. Create nrf52 workspace

```bash
$ mkdir nrf52-workspace && cd nrf52-workspace
```

2. Import mbed-os to the directory

```bash
$ mbed import mbed-os
```

3. Create `.mbedignore` in `mbed-os` directory and add:

```
features/cellular/*
features/cryptocell/*
features/deprecated_warnings/*
features/device_key/*
features/FEATURE_BOOTLOADER/*
features/frameworks/*
features/lorawan/*
features/lwipstack/*
features/nanostack/*
features/netsocket/*
features/nfc/*
features/unsupported/*
features/storage/*
components/wifi/*
components/802.15.4_RF/*
components/storage/*
components/TARGET_PSA/*
usb/*
```

4. Set path to mbed-os in Mbed CLI

```bash
$ mbed config -G MBED_OS_DIR <path>/nrf52-workspace/mbed-os
```

5. Import template

```bash
$ git clone https://github.com/byq77/walking-leds-template-mbed.git
```

6. Set path to `arm-none-eabi-gcc` in `settings.json`
```json
{
    "C_Cpp.default.compilerPath": "C:\\Program Files (x86)\\GNU Tools ARM Embedded\\6 2017-q2-update\\bin\\arm-none-eabi-gcc.exe",
}
```

## Tasks

To compile/flash:
* `CTRL + SHIFT + P`, type `Tasks: Run Task` and select task.

## Debug

To debug:
* install extension: https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug
* compile and flash DEBUG firmware
* `CTRL + SHIFT + D` and click on `start debug` button