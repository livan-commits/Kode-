/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include "kode.h"

KODE_APP_MANIFEST("Haptic Studio", "maker");

static const char *profiles[] = {
    "click", "tick", "double", "success", "warning", "error", "heartbeat"
};
static const char *labels[] = {
    "Click", "Tick", "Double", "Success", "Warning", "Error", "Heartbeat"
};
#define PROFILE_COUNT (sizeof(profiles) / sizeof(profiles[0]))

void app_main(void) {
    static const uint16_t heartbeat_pattern[] = {80U, 70U, 160U, 100U};
    uint32_t selected = 0U;
    bool custom_ready;
    kode_widget_t selected_widget, status_widget, play_button;

    kode_init();
    custom_ready = kode_haptic_add_profile("heartbeat", heartbeat_pattern,
        (uint32_t)(sizeof(heartbeat_pattern) / sizeof(heartbeat_pattern[0])), 70U);

    if (!kode_screen_new("Haptic Studio")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }
    selected_widget = kode_screen_add_text("Selected: Click");
    status_widget = kode_screen_add_text(custom_ready ?
        "Choose a pattern and press TOP or Play." :
        "Built-in patterns ready; custom pattern unavailable.");
    play_button = kode_screen_add_button("Play pattern");
    if (!selected_widget || !status_widget || !play_button) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    for (;;) {
        if (kode_button_was_pressed("bottom")) kode_exit();

        if (kode_button_was_pressed("up") && selected > 0U) --selected;
        if (kode_button_was_pressed("down") && selected + 1U < PROFILE_COUNT) ++selected;
        if (kode_button_was_pressed("up") || kode_button_was_pressed("down")) {
            (void)kode_widget_set_text(selected_widget, "Selected: %s", labels[selected]);
            (void)kode_widget_set_text(status_widget, "TOP or Play to test this vibration.");
        }

        if (kode_button_was_pressed("top") || kode_widget_was_pressed(play_button)) {
            if (kode_haptic_play(profiles[selected])) {
                (void)kode_widget_set_text(status_widget, "Playing: %s", labels[selected]);
            } else {
                (void)kode_widget_set_text(status_widget, "Haptic error: %.55s", kode_error());
            }
        }
        kode_delay(70U);
    }
}
