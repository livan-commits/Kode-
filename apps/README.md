# Native Kode Dot apps

These projects are complete ESP-IDF firmware images for the Kode Dot, not HTML mockups. They use the official Kode SDK and `kode_screen` standard UI API, pinned to v0.1.2.

| App | Directory | Controls | Offline |
|---|---|---|---|
| Stopwatch | `stopwatch/` | TOP start/pause, RIGHT lap, LEFT reset, BOT home | Yes |
| Pomodoro | `pomodoro/` | TOP start/pause, RIGHT next phase, LEFT reset, BOT home | Yes |
| Calculator | `calculator/` | D-pad edits operands, TOP changes operation, touch buttons reset/cycle | Yes |

Build each app from its directory with ESP-IDF v6.1:

```sh
. "$IDF_PATH/export.sh"
idf.py build
```

GitHub Actions builds all three projects from clean checkouts. A green build proves compilation against the pinned SDK, not correct behavior on the physical Dot. Hardware acceptance is still required.

| Dice | `dice/` | Roll button or TOP | Yes |
| Reaction Test | `reaction-test/` | Start, then press a direction after GO | Yes |
| Memory | `memory/` | Memorize and repeat the D-pad sequence | Yes |
| Snake | `snake/` | D-pad steers, TOP pauses, Restart resets | Yes |
| Pong | `pong/` | UP/DOWN moves paddle, TOP pauses, Serve starts | Yes |
| Quiz | `quiz/` | D-pad selects answers, TOP confirms | Yes |
| Tower Stack | `tower-stack/` | LEFT/RIGHT move the block; TOP or Drop places it; LEFT restarts after game over; BOT home | Yes |

| Tasks | `tasks/` | TOP marks/unmarks; UP/DOWN selects; LEFT resets; BOT home | Yes, session only |
| Shopping List | `shopping-list/` | TOP toggles item; UP/DOWN selects; LEFT resets; BOT home | Yes, session only |
| Habit Tracker | `habit-tracker/` | TOP checks/unchecks; UP/DOWN selects; LEFT resets; BOT home | Yes, session only |

| Alarm | `alarm/` | TOP start/pause; UP/DOWN adjusts one minute; LEFT resets; BOT home | Yes, foreground countdown only |
| Reminders | `reminders/` | TOP marks/unmarks; UP/DOWN selects; LEFT resets; BOT home | Yes, predefined list; completion saved in Kode settings |

**Alarm limitation:** this app is a foreground countdown. It does not register an operating-system alarm and cannot wake the Dot after the app exits. **Reminders limitation:** the current list is built in; completion state persists in the app settings, but users cannot yet create or edit reminder text or schedule background notifications.

| Calendar | `calendar/` | LEFT/RIGHT previous/next month; TOP today; BOT home | Yes, requires KodeOS clock |

| Compass | `compass/` | TOP calibrates; LEFT clears calibration; BOT home | Yes, needs magnetometer and accelerometer |

| Level | `level/` | Live pitch/roll; BOT home | Yes, requires IMU |
| Sensor Studio | `sensor-studio/` | Live IMU and magnetometer readings; BOT home | Yes, requires sensors |
| Data Logger | `data-logger/` | TOP starts/stops CSV logging to `flash/sensor-log.csv`; BOT home | Yes, requires IMU and flash storage |

| NFC Studio | `nfc-studio/` | Reads NFC UID and tag type; BOT home | Yes, requires NFC hardware |
| RFID Studio | `rfid-studio/` | Reads EM4100 IDs only; BOT home | Yes, requires RFID hardware |
| IR Studio | `ir-studio/` | Receives decoded IR protocol/address/command; BOT home | Yes, requires IR receiver |

| Battery Monitor | `battery-monitor/` | Live charge, voltage and charging status; BOT home | Yes, requires fuel gauge |
| LED Studio | `led-studio/` | Select/apply RGB LED presets; BOT home | Yes, requires RGB LED |
| Music Studio | `music-studio/` | Play selectable speaker tones; BOT home | Yes, requires audio codec |
| Files | `files/` | Read-only flash/SD file listing; BOT home | Yes, requires mounted storage |

| Device Diagnostics | `device-diagnostics/` | Device ID, uptime, battery, local clock and Wi-Fi/USB status; BOT home | Yes, clock may be unset |
| Network Monitor | `network-monitor/` | Wi-Fi state; TOP runs two ICMP probes to 1.1.1.1 | No, requires Wi-Fi and radio |
| Wi-Fi Analyzer | `wi-fi-analyzer/` | TOP scans nearby access points; about 38 seconds; BOT home | No, requires radio; scan may interrupt another Wi-Fi scan |
| Bluetooth Scanner | `bluetooth-scanner/` | TOP listens for BLE devices for 3 seconds; BOT home | No, requires Bluetooth/radio enabled |
| Settings | `settings/` | UP/DOWN brightness; LEFT/RIGHT volume; TOP mute; BOT home | Brightness preference saved; volume and mute are boot-session settings |

**Hardware and radio notes:** Wi-Fi and Bluetooth scans use the shared radio and can be refused while it is busy or disabled in KodeOS. A successful compile does not prove radio hardware availability. Device Diagnostics is read-only. Settings restores the display's previous brightness when it exits, as defined by the SDK; it stores its preferred level separately. Volume and mute last only for the current boot.

**Audio note:** Music Studio is a tone test tool, not a music player. Its selectable frequencies stay within the SDK's documented 400–3400 Hz range.
