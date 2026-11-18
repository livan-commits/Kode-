/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Presentation Remote", "control");

#define HID_KEY_ENTER    0x28U
#define HID_KEY_ESCAPE   0x29U
#define HID_KEY_SPACE    0x2CU
#define HID_KEY_PAGEUP   0x4BU
#define HID_KEY_PAGEDOWN 0x4EU

static kode_widget_t status_widget;

static void on_pair_code(const kode_event_t *event) {
    (void)kode_widget_set_text(status_widget, "Pairing code: %.6s", event->text);
}
static void on_pair_done(const kode_event_t *event) {
    (void)kode_widget_set_text(status_widget, "%s",
        event->value == 1 ? "Computer paired. Ready." : "Pairing was not completed.");
}
static void on_connected(const kode_event_t *event) {
    (void)event;
    (void)kode_widget_set_text(status_widget, "Computer connected.");
}
static void on_disconnected(const kode_event_t *event) {
    (void)event;
    (void)kode_widget_set_text(status_widget, "Computer disconnected.");
}

static void send_key(uint8_t key, const char *label) {
    if (!kode_ble_hid_ready()) {
        (void)kode_widget_set_text(status_widget, "Not connected. Pair a computer first.");
        return;
    }
    if (kode_ble_hid_tap(0U, key)) {
        (void)kode_widget_set_text(status_widget, "Sent: %s", label);
    } else {
        (void)kode_widget_set_text(status_widget, "Send failed: %.60s", kode_error());
    }
}

static void pair_computer(void) {
    if (kode_ble_pair_open()) {
        (void)kode_widget_set_text(status_widget, "Pairing window open (5 min). Check computer.");
    } else {
        (void)kode_widget_set_text(status_widget, "Pairing failed: %.60s", kode_error());
    }
}

void app_main(void) {
    /* HID mode must be selected before any other SDK call in this boot. */
    const bool hid_selected = kode_ble_hid_select(KODE_BLE_HID_KEYBOARD);
    kode_widget_t pair, previous, next, enter, escape;

    kode_init();
    if (!kode_screen_new("Presentation Remote")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }

    status_widget = kode_screen_add_text(
        hid_selected ? "Pair a computer to begin." : "Bluetooth HID unavailable.");
    pair = kode_screen_add_button("Pair computer");
    previous = kode_screen_add_button("Previous slide");
    next = kode_screen_add_button("Next slide");
    enter = kode_screen_add_button("Enter");
    escape = kode_screen_add_button("Escape");
    if (!status_widget || !pair || !previous || !next || !enter || !escape) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    if (hid_selected) {
        (void)kode_event_add_listener(KODE_EVENT_BLE_PAIR_CODE, on_pair_code);
        (void)kode_event_add_listener(KODE_EVENT_BLE_PAIR_DONE, on_pair_done);
        (void)kode_event_add_listener(KODE_EVENT_BLE_CONNECTED, on_connected);
        (void)kode_event_add_listener(KODE_EVENT_BLE_DISCONNECTED, on_disconnected);
    }

    for (;;) {
        if (kode_button_was_pressed("bottom")) kode_exit();
        if (kode_widget_was_pressed(pair)) pair_computer();
        if (kode_widget_was_pressed(previous) || kode_button_was_pressed("up") ||
            kode_button_was_pressed("left")) send_key(HID_KEY_PAGEUP, "previous slide");
        if (kode_widget_was_pressed(next) || kode_button_was_pressed("down") ||
            kode_button_was_pressed("right")) send_key(HID_KEY_PAGEDOWN, "next slide");
        if (kode_widget_was_pressed(enter)) send_key(HID_KEY_ENTER, "Enter");
        if (kode_widget_was_pressed(escape)) send_key(HID_KEY_ESCAPE, "Escape");
        if (kode_button_was_pressed("top")) send_key(HID_KEY_SPACE, "Space");
        kode_delay(60U);
    }
}
