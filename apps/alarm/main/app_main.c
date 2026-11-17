/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Alarm", "life");

void app_main(void)
{
    kode_widget_t value, state, start_button, up_button, down_button, reset_button;
    uint32_t remaining_ms = 5U * 60000U, started_at = 0U;
    bool running = false, fired = false;
    char display[24];

    kode_init();
    if (!kode_screen_new("Alarm")) { kode_print("screen failed: %s", kode_error()); kode_exit(); }
    value = kode_screen_add_value("05:00", "foreground countdown");
    state = kode_screen_add_text("Ready - TOP to start");
    start_button = kode_screen_add_button("Start / Pause");
    up_button = kode_screen_add_button("+ 1 minute");
    down_button = kode_screen_add_button("- 1 minute");
    reset_button = kode_screen_add_button("Reset to 5 min");
    if (!value || !state || !start_button || !up_button || !down_button || !reset_button) {
        kode_print("widget creation failed: %s", kode_error()); kode_exit();
    }

    for (;;) {
        uint32_t now = kode_uptime_ms();
        bool start = kode_widget_was_pressed(start_button) || kode_button_was_pressed("top");
        bool up = kode_widget_was_pressed(up_button) || kode_button_was_pressed("up");
        bool down = kode_widget_was_pressed(down_button) || kode_button_was_pressed("down");
        bool reset = kode_widget_was_pressed(reset_button) || kode_button_was_pressed("left");
        if (kode_button_was_pressed("bottom")) { kode_exit(); }
        if (running) {
            uint32_t elapsed = now - started_at;
            if (elapsed >= remaining_ms) {
                remaining_ms = 0U; running = false; fired = true;
                (void)kode_haptic_play("success");
            } else {
                remaining_ms -= elapsed; started_at = now;
            }
        }
        if (start && remaining_ms > 0U) {
            running = !running;
            if (running) { started_at = now; fired = false; }
        }
        if (!running && up && remaining_ms <= (59U * 60000U)) {
            remaining_ms += 60000U; fired = false;
        }
        if (!running && down && remaining_ms >= 60000U) {
            remaining_ms -= 60000U; fired = false;
        }
        if (reset) { remaining_ms = 5U * 60000U; running = false; fired = false; }
        (void)snprintf(display, sizeof(display), "%02lu:%02lu",
            (unsigned long)(remaining_ms / 60000U),
            (unsigned long)((remaining_ms / 1000U) % 60U));
        (void)kode_widget_set_text(value, "%s", display);
        (void)kode_widget_set_text(state, "%s", fired ? "Finished - app must stay open" : (running ? "Counting down" : "Paused / ready"));
        kode_delay(100U);
    }
}
