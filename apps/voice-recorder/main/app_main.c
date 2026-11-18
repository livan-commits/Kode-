/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Voice Recorder", "media");

#define MAX_RECORD_MS 30000U
#define RECORD_PATH "flash/voice-note.wav"

static void show(kode_widget_t status, const char *message) {
    (void)kode_widget_set_text(status, "%s", message);
}

void app_main(void) {
    kode_widget_t state, record, save;
    bool active = false;
    char line[KODE_WIDGET_TEXT_MAX];

    kode_init();
    if (!kode_screen_new("Voice Recorder")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }

    state = kode_screen_add_text("Ready. Record up to 30 seconds.");
    record = kode_screen_add_button("Start recording");
    save = kode_screen_add_button("Save WAV");
    if (!state || !record || !save) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    for (;;) {
        if (kode_button_was_pressed("bottom")) {
            if (active) (void)kode_audio_record_stop();
            kode_exit();
        }

        if (kode_widget_was_pressed(record)) {
            if (!active) {
                if (kode_audio_record_start(MAX_RECORD_MS)) {
                    active = true;
                    (void)kode_widget_set_text(record, "Stop recording");
                    show(state, "Recording... max 30 seconds");
                } else {
                    (void)snprintf(line, sizeof(line), "Microphone unavailable: %s", kode_error());
                    show(state, line);
                }
            } else if (kode_audio_record_stop()) {
                active = false;
                (void)kode_widget_set_text(record, "Start recording");
                show(state, "Recording stopped. Save WAV to keep it.");
            } else {
                (void)snprintf(line, sizeof(line), "Stop failed: %s", kode_error());
                show(state, line);
            }
        }

        if (active) {
            kode_audio_record_status_t s = kode_audio_record_status();
            if (!s.recording) {
                active = false;
                (void)kode_widget_set_text(record, "Start recording");
                (void)snprintf(line, sizeof(line), "Finished: %lu ms. Press Save WAV.",
                               (unsigned long)s.ms);
                show(state, line);
            } else {
                (void)snprintf(line, sizeof(line), "Recording: %lu / 30000 ms",
                               (unsigned long)s.ms);
                show(state, line);
            }
        }

        if (kode_widget_was_pressed(save)) {
            if (active) {
                show(state, "Stop recording before saving.");
            } else {
                kode_audio_record_status_t s = kode_audio_record_status();
                if (s.frames == 0U) {
                    show(state, "No recording to save yet.");
                } else if (kode_audio_record_save(RECORD_PATH)) {
                    show(state, "Saved: flash/voice-note.wav");
                } else {
                    (void)snprintf(line, sizeof(line), "Save failed: %s", kode_error());
                    show(state, line);
                }
            }
        }
        kode_delay(50U);
    }
}
