/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdio.h>
#include "kode.h"
KODE_APP_MANIFEST("WAV Player", "media");
#define AUDIO_PATH "flash/voice-note.wav"
static void status(kode_widget_t widget, const char *message) {
    (void)kode_widget_set_text(widget, "%s", message);
}
void app_main(void) {
    kode_widget_t state, play;
    bool playing = false;
    char message[KODE_WIDGET_TEXT_MAX];
    kode_init();
    if (!kode_screen_new("WAV Player")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }
    state = kode_screen_add_text("Ready. Plays flash/voice-note.wav");
    play = kode_screen_add_button("Play WAV");
    if (!state || !play) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }
    for (;;) {
        if (kode_button_was_pressed("bottom")) {
            if (playing) kode_audio_play_stop();
            kode_exit();
        }
        if (kode_button_was_pressed("top") || kode_widget_was_pressed(play)) {
            if (playing || kode_audio_is_playing()) {
                kode_audio_play_stop();
                playing = false;
                (void)kode_widget_set_text(play, "Play WAV");
                status(state, "Playback stopped.");
            } else if (kode_audio_play_start(AUDIO_PATH)) {
                playing = true;
                (void)kode_widget_set_text(play, "Stop playback");
                status(state, "Playing WAV...");
            } else {
                (void)snprintf(message, sizeof(message), "Cannot play: %.80s", kode_error());
                status(state, message);
            }
        }
        if (playing && !kode_audio_is_playing()) {
            playing = false;
            (void)kode_widget_set_text(play, "Play WAV");
            status(state, "Playback finished.");
        }
        kode_delay(50U);
    }
}
