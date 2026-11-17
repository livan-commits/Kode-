# Stopwatch

A native Kode Dot application built against the official Kode SDK v0.1.2. This is source code for the ESP32-P4 app image, not a browser mockup.

## Build

Use ESP-IDF v6.1, then:

```sh
. "$IDF_PATH/export.sh"
idf.py build
```

The first configure fetches the board settings and SDK components. The app uses the standard `kode.h` API and the official `kode_screen` screen widgets. It has not been hardware-tested yet; a successful CI build is the next acceptance gate.

## Controls

TOP or Start/Pause starts and pauses; RIGHT or Lap records a lap; LEFT or Reset clears the time; BOT returns to KodeOS.

## SDK and licence

SDK dependency: `kodediy/kode-sdk` v0.1.2. Licence: GPL-3.0-or-later, following the SDK app template.
