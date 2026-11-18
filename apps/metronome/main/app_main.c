/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Metronome", "media");

#define BPM_MIN 40U
#define BPM_MAX 240U
#define BPM_STEP 5U
#define BEAT_MS 45U

static void update_tempo(kode_widget_t widget, uint32_t bpm) {
    (void)kode_widget_set_text(widget, "%lu BPM", (unsigned long)bpm);
}

void app_main(void) {
    kode_widget_t tempo_widget, state, toggle;
    uint32_t bpm = 120U;
    uint32_t next_beat = 0U;
    uint8_t beat = 0U;
    bool running = false;
    char message[KODE_WIDGET_TEXT_MAX];

    kode_init();
    if (!kode_screen_new("Metronome")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }
    tempo_widget = kode_screen_add_value("120", "BPM");
    state = kode_screen_add_text("UP/DOWN tempo; TOP start/stop");
    toggle = kode_screen_add_button("Start metronome");
    if (!tempo_widget || !state || !toggle) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }
    for (;;) {
        if (kode_button_was_pressed("bottom")) kode_exit();
        if (kode_button_was_pressed("up") && bpm < BPM_MAX) { bpm += BPM_STEP; update_tempo(tempo_widget, bpm); }
        if (kode_button_was_pressed("down") && bpm > BPM_MIN) { bpm -= BPM_STEP; update_tempo(tempo_widget, bpm); }
        if (kode_button_was_pressed("top") || kode_widget_was_pressed(toggle)) {
            running = !running;
            if (running) {
                beat = 0U;
                next_beat = kode_uptime_ms();
                (void)kode_widget_set_text(toggle, "Stop metronome");
                (void)kode_widget_set_text(state, "Running - first beat accented");
            } else {
                (void)kode_widget_set_text(toggle, "Start metronome");
                (void)kode_widget_set_text(state, "Stopped");
            }
        }
        if (running) {
            uint32_t now = kode_uptime_ms();
            if ((int32_t)(now - next_beat) >= 0) {
                uint32_t interval = 60000U / bpm;
                beat = (uint8_t)((beat % 4U) + 1U);
                if (!kode_audio_tone(beat == 1U ? 1200U : 700U, BEAT_MS)) {
                    (void)snprintf(message, sizeof(message), "Audio unavailable: %.70s", kode_error());
                    (void)kode_widget_set_text(state, "%s", message);
                    running = false;
                    (void)kode_widget_set_text(toggle, "Start metronome");
                } else {
                    (void)snprintf(message, sizeof(message), "Beat %u/4 - %lu BPM", (unsigned)beat, (unsigned long)bpm);
                    (void)kode_widget_set_text(state, "%s", message);
                    next_beat = now + interval - BEAT_MS;
                }
            }
        }
        kode_delay(5U);
    }
}
