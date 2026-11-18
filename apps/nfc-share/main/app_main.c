/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include "kode.h"

KODE_APP_MANIFEST("NFC Share", "maker");

static const char *const labels[] = {"Share demo text", "Share demo link"};
static const char *const types[] = {"text", "uri"};
static const char *const payloads[] = {
    "Hello from Kode Dot",
    "https://example.com/kode-dot"
};

static bool register_profile(uint32_t selected) {
    return kode_nfc_add_profile("share", types[selected], payloads[selected]);
}

void app_main(void) {
    uint32_t selected = 0U;
    bool active = false;
    kode_widget_t selection, status, toggle;

    kode_init();
    if (!kode_screen_new("NFC Share")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }

    selection = kode_screen_add_text("Share demo text");
    status = kode_screen_add_text("Only safe NDEF text/URI profiles are used.");
    toggle = kode_screen_add_button("Start sharing");
    if (!selection || !status || !toggle) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    if (!register_profile(selected)) {
        (void)kode_widget_set_text(status, "Profile setup failed: %.45s", kode_error());
    }

    for (;;) {
        if (kode_button_was_pressed("bottom")) {
            if (active) (void)kode_nfc_emulate_stop();
            kode_exit();
        }

        if (!active && kode_button_was_pressed("up") && selected > 0U) {
            --selected;
            if (!register_profile(selected)) {
                (void)kode_widget_set_text(status, "Profile setup failed: %.45s", kode_error());
            } else {
                (void)kode_widget_set_text(selection, "%s", labels[selected]);
                (void)kode_widget_set_text(status, "TOP or tap to start sharing.");
            }
        }
        if (!active && kode_button_was_pressed("down") && selected == 0U) {
            ++selected;
            if (!register_profile(selected)) {
                (void)kode_widget_set_text(status, "Profile setup failed: %.45s", kode_error());
            } else {
                (void)kode_widget_set_text(selection, "%s", labels[selected]);
                (void)kode_widget_set_text(status, "TOP or tap to start sharing.");
            }
        }

        if (kode_button_was_pressed("top") || kode_widget_was_pressed(toggle)) {
            if (active) {
                if (kode_nfc_emulate_stop()) {
                    active = false;
                    (void)kode_widget_set_text(toggle, "Start sharing");
                    (void)kode_widget_set_text(status, "NFC sharing stopped.");
                } else {
                    (void)kode_widget_set_text(status, "Stop failed: %.45s", kode_error());
                }
            } else if (kode_nfc_emulate_start("share")) {
                active = true;
                (void)kode_widget_set_text(toggle, "Stop sharing");
                (void)kode_widget_set_text(status, "Active. Tap a phone to the Dot.");
            } else {
                (void)kode_widget_set_text(status, "Start failed: %.45s", kode_error());
            }
        }
        kode_delay(80U);
    }
}
