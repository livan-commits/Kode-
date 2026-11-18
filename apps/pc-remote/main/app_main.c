/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "kode.h"

KODE_APP_MANIFEST("PC Remote", "control");

static volatile bool s_pair_code_ready;
static volatile bool s_pair_done;
static char s_pair_code[16];

static void on_pair_code(const kode_event_t *event) {
    if (event && event->text) {
        (void)snprintf(s_pair_code, sizeof(s_pair_code), "%.15s", event->text);
        s_pair_code_ready = true;
    }
}
static void on_pair_done(const kode_event_t *event) {
    (void)event;
    s_pair_done = true;
}

void app_main(void) {
    bool hid_ok = kode_ble_hid_select(KODE_BLE_HID_MOUSE | KODE_BLE_HID_KEYBOARD);
    kode_widget_t status, pairing, code, right_click, enter_key, space_key;
    bool last_ready = false;
    uint32_t last_move = 0U;
    char line[KODE_WIDGET_TEXT_MAX];

    kode_init();
    if (!kode_screen_new("PC Remote")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }
    status = kode_screen_add_text(hid_ok ? "Bluetooth mouse/keyboard ready to pair" : "Bluetooth HID unavailable");
    pairing = kode_screen_add_button("Pair with PC");
    code = kode_screen_add_text("Pairing code: --");
    right_click = kode_screen_add_button("Right click");
    enter_key = kode_screen_add_button("Enter");
    space_key = kode_screen_add_button("Space");
    if (!status || !pairing || !code || !right_click || !enter_key || !space_key) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }
    (void)kode_event_add_listener(KODE_EVENT_BLE_PAIR_CODE, on_pair_code);
    (void)kode_event_add_listener(KODE_EVENT_BLE_PAIR_DONE, on_pair_done);
    (void)kode_widget_set_text(code, "Pair in PC Bluetooth settings");

    for (;;) {
        uint32_t now = kode_uptime_ms();
        bool ready;
        int8_t dx = 0, dy = 0;
        if (kode_button_was_pressed("bottom")) kode_exit();

        if (kode_widget_was_pressed(pairing)) {
            if (kode_ble_pair_open()) {
                (void)kode_widget_set_text(status, "Pairing window open for 5 minutes");
            } else {
                (void)snprintf(line, sizeof(line), "Pairing failed: %.90s", kode_error());
                (void)kode_widget_set_text(status, "%s", line);
            }
        }
        ready = hid_ok && kode_ble_hid_ready();
        if (ready != last_ready) {
            (void)kode_widget_set_text(status, ready ? "PC connected - D-pad moves mouse" : "PC disconnected - pair from Bluetooth settings");
            last_ready = ready;
        }
        if (s_pair_code_ready) {
            s_pair_code_ready = false;
            (void)kode_widget_set_text(code, "Pairing code: %s", s_pair_code);
        }
        if (s_pair_done) {
            s_pair_done = false;
            (void)kode_widget_set_text(status, ready ? "Pairing complete - PC connected" : "Pairing finished - waiting for PC");
        }

        if (ready && (uint32_t)(now - last_move) >= 30U) {
            if (kode_button_is_pressed("left")) dx -= 7;
            if (kode_button_is_pressed("right")) dx += 7;
            if (kode_button_is_pressed("up")) dy -= 7;
            if (kode_button_is_pressed("down")) dy += 7;
            if (dx || dy) (void)kode_ble_hid_mouse(0U, dx, dy, 0);
            last_move = now;
            if (kode_button_was_pressed("top")) (void)kode_ble_hid_click(0x01U);
            if (kode_widget_was_pressed(right_click)) (void)kode_ble_hid_click(0x02U);
            if (kode_widget_was_pressed(enter_key)) (void)kode_ble_hid_tap(0U, 0x28U);
            if (kode_widget_was_pressed(space_key)) (void)kode_ble_hid_tap(0U, 0x2CU);
        } else {
            if (ready && kode_button_was_pressed("top")) (void)kode_ble_hid_click(0x01U);
            if (ready && kode_widget_was_pressed(right_click)) (void)kode_ble_hid_click(0x02U);
            if (ready && kode_widget_was_pressed(enter_key)) (void)kode_ble_hid_tap(0U, 0x28U);
            if (ready && kode_widget_was_pressed(space_key)) (void)kode_ble_hid_tap(0U, 0x2CU);
        }
        kode_delay(15U);
    }
}
