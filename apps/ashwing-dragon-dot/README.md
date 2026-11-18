# Ashwing Dragon — Kode Dot

A native Kode Dot app that plays pre-rendered PNG frames from the user's original Ashwing Drake Lite GLB. The desktop pipeline renders the original mesh, textures, 61-joint skin and six embedded clips; the Dot displays those frames as a 2D sprite animation.

**Important:** this is not live 3D rendering on the Dot. The current documented ESP32-P4/Kode SDK stack has no supported hardware-accelerated glTF renderer. The desktop Three.js viewer remains the true interactive 3D experience.

## Assets

The app expects these directories on the root of the Kode Dot SD card:

```
ashwing/
  idle_loop/frame_000.png ... frame_023.png
  walk_loop/frame_000.png ... frame_008.png
  fly_loop/frame_000.png ... frame_006.png
  takeoff/frame_000.png ... frame_019.png
  roar/frame_000.png ... frame_015.png
  bite/frame_000.png ... frame_009.png
```

The PNG frames are derived from the original Ashwing GLB and are intended to be packaged with this app. Copy the supplied `ashwing/` folder to the SD card root. The app reports a clear error if the files are absent.

## Controls

- Touch **Previous** / **Next**, or use LEFT / RIGHT, to change animation.
- Touch **Play / Pause**, or press TOP, to pause or resume.
- BOT returns to KodeOS.

The six original clips are included: idle, walk, fly, takeoff, roar and bite. Playback is 8 frames per second (with per-clip timing rounded to the nearest millisecond).

## Build

Requires ESP-IDF v6.1 and the free Kode SDK v0.1.2:

```sh
. "$IDF_PATH/export.sh"
idf.py build
```

## Asset licence

Ashwing Drake Lite is licensed by Literock Studios for free use in end products, including games and apps. The licence permits modified/compiled/packaged assets inside an end product but forbids redistributing the model or modified asset as a standalone asset pack. Do not publish the original GLB or these frames as a separate asset collection. Credit: “Ashwing Drake by Literock Studios”.
