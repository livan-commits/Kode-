/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Habit Tracker", "tools");
#define ITEM_COUNT 5U

void app_main(void)
{
    static const char *const items[ITEM_COUNT] = {
        "Drink water",
        "Move for 10 min",
        "Read",
        "Go outside",
        "Sleep routine"
    };
    bool completed[ITEM_COUNT] = { false };
    uint32_t selected = 0U, i, done;
    kode_widget_t status, progress, previous, toggle, next, reset;
    char line[64];

    kode_init();
    if (!kode_screen_new("Habit Tracker")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }
    status = kode_screen_add_value(items[0], "Selected item");
    progress = kode_screen_add_text("0 / 5 completed");
    previous = kode_screen_add_button("Previous");
    toggle = kode_screen_add_button("Check / Undo");
    next = kode_screen_add_button("Next");
    reset = kode_screen_add_button("Reset day");
    if ((0U == status) || (0U == progress) || (0U == previous) ||
        (0U == toggle) || (0U == next) || (0U == reset)) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    for (;;) {
        bool changed = false;
        const bool reset_pressed = kode_widget_was_pressed(reset) || kode_button_was_pressed("left");
        const bool previous_pressed = kode_widget_was_pressed(previous) || kode_button_was_pressed("up");
        const bool next_pressed = kode_widget_was_pressed(next) || kode_button_was_pressed("down") || kode_button_was_pressed("right");
        const bool toggle_pressed = kode_widget_was_pressed(toggle) || kode_button_was_pressed("top");

        if (kode_button_was_pressed("bottom")) { kode_exit(); }
        if (reset_pressed) {
            for (i = 0U; i < ITEM_COUNT; ++i) { completed[i] = false; }
            changed = true;
        } else if (previous_pressed) {
            selected = (selected + ITEM_COUNT - 1U) % ITEM_COUNT;
            changed = true;
        }
        if (next_pressed) {
            selected = (selected + 1U) % ITEM_COUNT;
            changed = true;
        }
        if (toggle_pressed) {
            completed[selected] = !completed[selected];
            changed = true;
        }
        if (changed) {
            done = 0U;
            for (i = 0U; i < ITEM_COUNT; ++i) { if (completed[i]) { ++done; } }
            (void)snprintf(line, sizeof(line), "%s %s", completed[selected] ? "[x]" : "[ ]", items[selected]);
            (void)kode_widget_set_text(status, "%s", line);
            (void)snprintf(line, sizeof(line), "%lu / %u completed", (unsigned long)done, (unsigned int)ITEM_COUNT);
            (void)kode_widget_set_text(progress, "%s", line);
        }
        kode_delay(50U);
    }
}
