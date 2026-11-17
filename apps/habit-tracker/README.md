# Habit Tracker

Offline session checklist with 5 starter entries. Select an entry, mark it complete, and view the completion count.

Native ESP32-P4 application using the official Kode SDK v0.1.2 and Kode screen widgets. This is firmware source, not a browser mockup.

## Build

Use ESP-IDF v6.1 and run the ESP-IDF export script, then run idf.py build from this directory.

## Controls

- TOP: Check / Undo
- UP: previous entry
- DOWN or RIGHT: next entry
- LEFT: Reset day
- BOT: return to KodeOS

State is held in RAM for the current app session and resets when the app exits. Hardware behavior still requires testing on a physical Kode Dot; CI compilation alone does not prove hardware acceptance.
