/* SPDX-License-Identifier: GPL-3.0-or-later */
/**
 * @file app_main.c
 * @brief A 25/5 minute Pomodoro timer for the Kode Dot.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Pomodoro", "productivity");

#define WORK_MS  (25U * 60U * 1000U)
#define BREAK_MS (5U * 60U * 1000U)

void app_main(void)
{
    kode_widget_t value;
    kode_widget_t phase_text;
    kode_widget_t start_button;
    kode_widget_t skip_button;
    kode_widget_t reset_button;
    uint32_t remaining = WORK_MS;
    uint32_t started_at = 0U;
    bool work_phase = true;
    bool running = false;
    char text[24];

    kode_init();
    if (!kode_screen_new("Pomodoro")) { kode_print("screen failed: %s", kode_error()); kode_exit(); }
    value = kode_screen_add_value("25:00", "minutes remaining");
    phase_text = kode_screen_add_text("Work session");
    start_button = kode_screen_add_button("Start / Pause");
    skip_button = kode_screen_add_button("Next phase");
    reset_button = kode_screen_add_button("Reset");
    if ((0U == value) || (0U == phase_text) || (0U == start_button) || (0U == skip_button) || (0U == reset_button)) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    for (;;) {
        const uint32_t now = kode_uptime_ms();
        const bool start = kode_widget_was_pressed(start_button) || kode_button_was_pressed("top");
        const bool skip = kode_widget_was_pressed(skip_button) || kode_button_was_pressed("right");
        const bool reset = kode_widget_was_pressed(reset_button) || kode_button_was_pressed("left");

        if (kode_button_was_pressed("bottom")) { kode_exit(); }
        if (start) {
            if (running) { remaining -= now - started_at; running = false; }
            else { started_at = now; running = true; }
        }
        if (skip) {
            work_phase = !work_phase;
            remaining = work_phase ? WORK_MS : BREAK_MS;
            started_at = now;
            running = false;
            (void)kode_haptic_play("double");
        }
        if (reset) {
            work_phase = true;
            remaining = WORK_MS;
            running = false;
            started_at = now;
        }
        if (running) {
            const uint32_t elapsed = now - started_at;
            if (elapsed >= remaining) {
                work_phase = !work_phase;
                remaining = work_phase ? WORK_MS : BREAK_MS;
                started_at = now;
                (void)kode_haptic_play("success");
            } else {
                remaining -= elapsed;
                started_at = now;
            }
        }
        (void)kode_widget_set_text(phase_text, "%s%s",
            work_phase ? "Work session" : "Break",
            running ? " - running" : " - paused");
        (void)snprintf(text, sizeof(text), "%02lu:%02lu",
            (unsigned long)(remaining / 60000U),
            (unsigned long)((remaining / 1000U) % 60U));
        (void)kode_widget_set_text(value, "%s", text);
        kode_delay(100U);
    }
}
