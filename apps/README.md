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

| Tasks | `tasks/` | TOP marks/unmarks; UP/DOWN selects; LEFT resets; BOT home | Yes, session only |
| Shopping List | `shopping-list/` | TOP toggles item; UP/DOWN selects; LEFT resets; BOT home | Yes, session only |
| Habit Tracker | `habit-tracker/` | TOP checks/unchecks; UP/DOWN selects; LEFT resets; BOT home | Yes, session only |

| Alarm | `alarm/` | TOP start/pause; UP/DOWN adjusts one minute; LEFT resets; BOT home | Yes, foreground countdown only |
| Reminders | `reminders/` | TOP marks/unmarks; UP/DOWN selects; LEFT resets; BOT home | Yes, predefined list; completion saved in Kode settings |

**Alarm limitation:** this app is a foreground countdown. It does not register an operating-system alarm and cannot wake the Dot after the app exits. **Reminders limitation:** the current list is built in; completion state persists in the app settings, but users cannot yet create or edit reminder text or schedule background notifications.

| Calendar | `calendar/` | LEFT/RIGHT previous/next month; TOP today; BOT home | Yes, requires KodeOS clock |

| Compass | `compass/` | TOP calibrates; LEFT clears calibration; BOT home | Yes, needs magnetometer and accelerometer |
