/* SPDX-License-Identifier: GPL-3.0-or-later */
/**
 * @file app_main.c
 * @brief Ashwing: pre-rendered frames from the original rigged GLB.
 *
 * The Kode Dot does not run a live glTF renderer. It plays PNG frames rendered
 * from the exact Ashwing GLB on a desktop, preserving the six source clips as
 * a 2D sprite animation. Frames are read from the SD card.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Ashwing Dragon", "play");

#define CANVAS_W 330U
#define CANVAS_H 190U
#define SPRITE_X 85
#define SPRITE_Y 12

typedef struct {
    const char *name;
    const char *label;
    uint16_t frames;
    uint16_t frame_ms;
} ashwing_clip_t;

static const ashwing_clip_t CLIPS[] = {
    { "idle_loop", "IDLE", 24U, 125U },
    { "walk_loop", "WALK", 9U, 118U },
    { "fly_loop", "FLY", 7U, 114U },
    { "takeoff", "TAKEOFF", 20U, 120U },
    { "roar", "ROAR", 16U, 125U },
    { "bite", "BITE", 10U, 120U }
};
#define CLIP_COUNT ((uint8_t)(sizeof(CLIPS) / sizeof(CLIPS[0])))

static bool draw_frame(kode_canvas_t canvas, uint8_t clip, uint16_t frame)
{
    char path[KODE_FILE_PATH_MAX];
    const int n = snprintf(path, sizeof(path), "sd/ashwing/%s/frame_%03u.png",
                           CLIPS[clip].name, (unsigned)frame);
    if ((n <= 0) || ((size_t)n >= sizeof(path))) {
        return false;
    }

    if (!kode_canvas_fill(canvas, kode_rgb(9U, 14U, 24U))) {
        return false;
    }
    if (!kode_draw_png(canvas, SPRITE_X, SPRITE_Y, path)) {
        return false;
    }
    return true;
}

static void show_state(kode_widget_t state, uint8_t clip, uint16_t frame, bool playing)
{
    (void)kode_widget_set_text(state, "ASHWING  /  %s   %02u/%02u   %s",
                               CLIPS[clip].label, (unsigned)(frame + 1U),
                               (unsigned)CLIPS[clip].frames,
                               playing ? "PLAYING" : "PAUSED");
}

void app_main(void)
{
    kode_canvas_t canvas;
    kode_widget_t state;
    kode_widget_t previous;
    kode_widget_t next;
    kode_widget_t toggle;
    uint8_t clip = 0U;
    uint16_t frame = 0U;
    uint32_t last_frame = 0U;
    bool playing = true;
    bool frame_ok;

    kode_init();
    if (!kode_screen_new("Ashwing Dragon")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }

    canvas = kode_screen_add_canvas(CANVAS_W, CANVAS_H);
    state = kode_screen_add_text("Loading Ashwing...");
    previous = kode_screen_add_button("Previous");
    next = kode_screen_add_button("Next");
    toggle = kode_screen_add_button("Play / Pause");
    if ((!canvas) || (!state) || (!previous) || (!next) || (!toggle)) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    (void)kode_widget_set_color(previous, kode_rgb(65U, 105U, 150U));
    (void)kode_widget_set_color(next, kode_rgb(65U, 105U, 150U));
    (void)kode_widget_set_color(toggle, kode_rgb(65U, 145U, 115U));

    frame_ok = draw_frame(canvas, clip, frame);
    if (!frame_ok) {
        char error[KODE_WIDGET_TEXT_MAX];
        (void)snprintf(error, sizeof(error),
                       "Missing SD sprites: copy the ashwing folder to SD root. (%s)",
                       kode_error());
        (void)kode_widget_set_text(state, "%s", error);
        playing = false;
    } else {
        show_state(state, clip, frame, playing);
    }

    last_frame = kode_uptime_ms();
    for (;;) {
        const uint32_t now = kode_uptime_ms();
        bool changed_clip = false;

        if (kode_button_was_pressed("bottom")) {
            kode_exit();
        }
        if (kode_widget_was_pressed(previous) || kode_button_was_pressed("left")) {
            clip = (uint8_t)((clip + CLIP_COUNT - 1U) % CLIP_COUNT);
            changed_clip = true;
        }
        if (kode_widget_was_pressed(next) || kode_button_was_pressed("right")) {
            clip = (uint8_t)((clip + 1U) % CLIP_COUNT);
            changed_clip = true;
        }
        if (kode_widget_was_pressed(toggle) || kode_button_was_pressed("top")) {
            playing = !playing;
            last_frame = now;
            show_state(state, clip, frame, playing);
        }

        if (changed_clip) {
            frame = 0U;
            last_frame = now;
            frame_ok = draw_frame(canvas, clip, frame);
            if (!frame_ok) {
                char error[KODE_WIDGET_TEXT_MAX];
                (void)snprintf(error, sizeof(error),
                               "Sprite read failed: %s", kode_error());
                (void)kode_widget_set_text(state, "%s", error);
                playing = false;
            } else {
                show_state(state, clip, frame, playing);
            }
        } else if (playing && ((uint32_t)(now - last_frame) >= CLIPS[clip].frame_ms)) {
            frame = (uint16_t)((frame + 1U) % CLIPS[clip].frames);
            last_frame = now;
            frame_ok = draw_frame(canvas, clip, frame);
            if (!frame_ok) {
                char error[KODE_WIDGET_TEXT_MAX];
                (void)snprintf(error, sizeof(error),
                               "Sprite read failed: %s", kode_error());
                (void)kode_widget_set_text(state, "%s", error);
                playing = false;
            } else {
                show_state(state, clip, frame, playing);
            }
        }
        kode_delay(10U);
    }
}
