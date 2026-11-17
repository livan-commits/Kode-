/* SPDX-License-Identifier: GPL-3.0-or-later */
/**
 * @file app_main.c
 * @brief A touch- and button-operated stopwatch for the Kode Dot.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Stopwatch", "tools");

static uint32_t elapsed_ms(uint32_t now, uint32_t accumulated, uint32_t started, bool running)
{
    return accumulated + (running ? now - started : 0U);
}

void app_main(void)
{
    kode_widget_t value;
    kode_widget_t state;
    kode_widget_t start_button;
    kode_widget_t lap_button;
    kode_widget_t reset_button;
    uint32_t accumulated = 0U;
    uint32_t started_at = 0U;
    uint32_t previous_lap = 0U;
    bool running = false;
    char text[48];

    kode_init();
    if (!kode_screen_new("Stopwatch")) { kode_print("screen failed: %s", kode_error()); kode_exit(); }
    value = kode_screen_add_value("00:00.0", "minutes : seconds . tenths");
    state = kode_screen_add_text("Paused");
    start_button = kode_screen_add_button("Start / Pause");
    lap_button = kode_screen_add_button("Lap");
    reset_button = kode_screen_add_button("Reset");
    if ((0U == value) || (0U == state) || (0U == start_button) || (0U == lap_button) || (0U == reset_button)) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    for (;;) {
        const uint32_t now = kode_uptime_ms();
        const uint32_t elapsed = elapsed_ms(now, accumulated, started_at, running);
        const bool start = kode_widget_was_pressed(start_button) || kode_button_was_pressed("top");
        const bool lap = kode_widget_was_pressed(lap_button) || kode_button_was_pressed("right");
        const bool reset = kode_widget_was_pressed(reset_button) || kode_button_was_pressed("left");

        if (kode_button_was_pressed("bottom")) { kode_exit(); }
        if (start) {
            if (running) { accumulated = elapsed; running = false; }
            else { started_at = now; running = true; }
            (void)kode_widget_set_text(state, "%s", running ? "Running" : "Paused");
        }
        if (lap && running) {
            const uint32_t lap_time = elapsed - previous_lap;
            previous_lap = elapsed;
            (void)snprintf(text, sizeof(text), "Lap %02lu:%02lu.%1lu",
                (unsigned long)(lap_time / 60000U),
                (unsigned long)((lap_time / 1000U) % 60U),
                (unsigned long)((lap_time / 100U) % 10U));
            (void)kode_widget_set_text(state, "%s", text);
        }
        if (reset) {
            accumulated = 0U;
            started_at = now;
            previous_lap = 0U;
            running = false;
            (void)kode_widget_set_text(state, "Paused");
        }
        (void)snprintf(text, sizeof(text), "%02lu:%02lu.%1lu",
            (unsigned long)(elapsed / 60000U),
            (unsigned long)((elapsed / 1000U) % 60U),
            (unsigned long)((elapsed / 100U) % 10U));
        (void)kode_widget_set_text(value, "%s", text);
        kode_delay(50U);
    }
}
