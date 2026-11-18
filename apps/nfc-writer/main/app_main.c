/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("NFC Writer", "maker");

typedef struct {
    const char *label;
    const char *type;
    const char *data;
} nfc_preset_t;

static const nfc_preset_t presets[] = {
    {"Demo text", "text", "Hello from Kode Dot"},
    {"Test text", "text", "Kode Dot NFC demo tag"},
    {"Demo link", "uri", "https://example.com/kode-dot"}
};
#define PRESET_COUNT (sizeof(presets) / sizeof(presets[0]))

static void update_selection(kode_widget_t selection_widget, uint32_t selected) {
    char text[120];
    (void)snprintf(text, sizeof(text), "%s: %s",
        presets[selected].label, presets[selected].data);
    (void)kode_widget_set_text(selection_widget, "%s", text);
}

void app_main(void) {
    uint32_t selected = 0U;
    bool confirm_write = false;
    kode_widget_t selection_widget, status_widget, write_button;

    kode_init();
    if (!kode_screen_new("NFC Writer")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }

    selection_widget = kode_screen_add_text("Demo text: Hello from Kode Dot");
    status_widget = kode_screen_add_text("Writes NDEF text/URI only. No badge IDs.");
    write_button = kode_screen_add_button("Write selected");
    if (!selection_widget || !status_widget || !write_button) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    for (;;) {
        if (kode_button_was_pressed("bottom")) kode_exit();

        if (kode_button_was_pressed("up") && selected > 0U) {
            --selected;
            confirm_write = false;
            update_selection(selection_widget, selected);
            (void)kode_widget_set_text(write_button, "Write selected");
            (void)kode_widget_set_text(status_widget, "Select a demo payload; TOP or tap to write.");
        }
        if (kode_button_was_pressed("down") && selected + 1U < PRESET_COUNT) {
            ++selected;
            confirm_write = false;
            update_selection(selection_widget, selected);
            (void)kode_widget_set_text(write_button, "Write selected");
            (void)kode_widget_set_text(status_widget, "Select a demo payload; TOP or tap to write.");
        }

        if (kode_button_was_pressed("top") || kode_widget_was_pressed(write_button)) {
            if (!confirm_write) {
                confirm_write = true;
                (void)kode_widget_set_text(write_button, "Confirm write");
                (void)kode_widget_set_text(status_widget,
                    "Confirm to write this public text/link to a compatible NFC tag.");
            } else {
                (void)kode_widget_set_text(status_widget, "Hold a compatible NFC tag near the Dot...");
                if (kode_nfc_write(presets[selected].type, presets[selected].data, 8000U)) {
                    (void)kode_widget_set_text(status_widget, "NFC tag written successfully.");
                } else {
                    (void)kode_widget_set_text(status_widget, "Write failed: %.55s", kode_error());
                }
                confirm_write = false;
                (void)kode_widget_set_text(write_button, "Write selected");
            }
        }
        kode_delay(80U);
    }
}
