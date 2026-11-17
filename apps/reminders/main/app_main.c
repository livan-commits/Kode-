/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Reminders", "life");
#define ITEM_COUNT 5U

void app_main(void)
{
    static const char *const items[ITEM_COUNT] = {
        "Drink some water", "Stand up and stretch", "Check today's priorities",
        "Take a screen break", "Prepare for tomorrow"
    };
    bool done[ITEM_COUNT] = { false };
    uint32_t selected = 0U, i, count;
    kode_widget_t current, progress, previous, toggle, next, reset;
    char line[64];

    kode_init();
    if (!kode_screen_new("Reminders")) { kode_print("screen failed: %s", kode_error()); kode_exit(); }
    current = kode_screen_add_value(items[0], "Selected reminder");
    progress = kode_screen_add_text("0 / 5 completed - session only");
    previous = kode_screen_add_button("Previous");
    toggle = kode_screen_add_button("Done / Undo");
    next = kode_screen_add_button("Next");
    reset = kode_screen_add_button("Reset");
    if (!current || !progress || !previous || !toggle || !next || !reset) {
        kode_print("widget creation failed: %s", kode_error()); kode_exit();
    }

    for (;;) {
        bool changed = false;
        bool prev = kode_widget_was_pressed(previous) || kode_button_was_pressed("up");
        bool advance = kode_widget_was_pressed(next) || kode_button_was_pressed("down") || kode_button_was_pressed("right");
        bool mark = kode_widget_was_pressed(toggle) || kode_button_was_pressed("top");
        bool clear = kode_widget_was_pressed(reset) || kode_button_was_pressed("left");
        if (kode_button_was_pressed("bottom")) { kode_exit(); }
        if (clear) { for (i = 0U; i < ITEM_COUNT; ++i) done[i] = false; changed = true; }
        else if (prev) { selected = (selected + ITEM_COUNT - 1U) % ITEM_COUNT; changed = true; }
        if (advance) { selected = (selected + 1U) % ITEM_COUNT; changed = true; }
        if (mark) { done[selected] = !done[selected]; changed = true; }
        if (changed) {
            count = 0U;
            for (i = 0U; i < ITEM_COUNT; ++i) if (done[i]) ++count;
            (void)snprintf(line, sizeof(line), "%s %s", done[selected] ? "[x]" : "[ ]", items[selected]);
            (void)kode_widget_set_text(current, "%s", line);
            (void)snprintf(line, sizeof(line), "%lu / %u completed - session only", (unsigned long)count, (unsigned int)ITEM_COUNT);
            (void)kode_widget_set_text(progress, "%s", line);
        }
        kode_delay(50U);
    }
}
