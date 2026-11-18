/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include "kode.h"

KODE_APP_MANIFEST("Compass Calibration", "maker");

static void draw_screen(kode_widget_t *status, kode_widget_t *calibrate,
                        const char *message) {
    if (!kode_screen_new("Compass Calibration")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }
    *status = kode_screen_add_text(message);
    *calibrate = kode_screen_add_button("Calibrate compass");
    if (!*status || !*calibrate) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }
}

void app_main(void) {
    kode_widget_t status, calibrate;
    const char *message;

    kode_init();
    message = kode_mag_is_calibrated() ?
        "Compass calibration is stored." :
        "Compass needs calibration.";
    draw_screen(&status, &calibrate, message);

    for (;;) {
        if (kode_button_was_pressed("bottom")) kode_exit();
        if (kode_button_was_pressed("top") || kode_widget_was_pressed(calibrate)) {
            const bool ok = kode_mag_calibrate();
            message = ok ? "Calibration saved successfully." :
                kode_error();
            /* The SDK calibration assistant replaces the current screen. */
            draw_screen(&status, &calibrate, message);
        }
        kode_delay(80U);
    }
}
