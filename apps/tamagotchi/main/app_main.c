/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Tamagotchi", "play");

#define DECAY_MS 60000U
#define CANVAS_W 330U
#define CANVAS_H 190U

static int clamp_stat(int value) {
    if (value < 0) return 0;
    if (value > 100) return 100;
    return value;
}

static void stroke(kode_canvas_t c, int x0, int y0, int x1, int y1,
                   int width, kode_color_t color) {
    int half = width / 2;
    for (int offset = -half; offset <= half; ++offset) {
        (void)kode_draw_line(c, x0 + offset, y0, x1 + offset, y1, color);
        if (offset != 0)
            (void)kode_draw_line(c, x0, y0 + offset, x1, y1 + offset, color);
    }
}

static void draw_dragon(kode_canvas_t c, int hunger, int happiness, int cleanliness) {
    const kode_color_t night = kode_rgb(12, 20, 38);
    const kode_color_t green = kode_rgb(48, 190, 137);
    const kode_color_t light_green = kode_rgb(112, 235, 178);
    const kode_color_t dark_green = kode_rgb(22, 112, 91);
    const kode_color_t wing = kode_rgb(105, 91, 190);
    const kode_color_t wing_light = kode_rgb(174, 139, 239);
    const kode_color_t gold = kode_rgb(255, 190, 75);
    const kode_color_t belly = kode_rgb(255, 218, 139);
    const kode_color_t ink = kode_rgb(17, 30, 42);
    const kode_color_t white = kode_rgb(255, 255, 255);
    const kode_color_t blush = kode_rgb(255, 132, 139);
    const int eye_y = happiness < 20 ? 75 : 70;

    (void)kode_canvas_fill(c, night);

    /* Soft stars and a small moon make the scene feel like a game screen. */
    (void)kode_fill_circle(c, 38, 28, 2, gold);
    (void)kode_fill_circle(c, 286, 34, 2, white);
    (void)kode_fill_circle(c, 305, 116, 1, gold);
    (void)kode_fill_circle(c, 58, 145, 1, white);
    (void)kode_draw_circle(c, 278, 65, 15, kode_rgb(75, 105, 147));
    (void)kode_fill_circle(c, 284, 60, 13, night);

    /* Wings behind the body: clean, symmetrical, one-pixel outlines. */
    (void)kode_fill_circle(c, 117, 78, 24, wing);
    stroke(c, 127, 91, 68, 27, 4, wing_light);
    stroke(c, 127, 91, 93, 43, 3, wing_light);
    stroke(c, 68, 27, 74, 78, 3, wing_light);
    stroke(c, 74, 78, 127, 91, 3, wing_light);
    stroke(c, 92, 43, 103, 79, 2, wing_light);
    stroke(c, 103, 79, 127, 91, 2, wing_light);

    (void)kode_fill_circle(c, 211, 78, 24, wing);
    stroke(c, 201, 91, 260, 27, 4, wing_light);
    stroke(c, 201, 91, 235, 43, 3, wing_light);
    stroke(c, 260, 27, 254, 78, 3, wing_light);
    stroke(c, 254, 78, 201, 91, 3, wing_light);
    stroke(c, 236, 43, 225, 79, 2, wing_light);
    stroke(c, 225, 79, 201, 91, 2, wing_light);

    /* Tail, feet and rounded body. */
    stroke(c, 154, 125, 123, 143, 15, dark_green);
    stroke(c, 123, 143, 104, 132, 10, green);
    (void)kode_fill_circle(c, 168, 121, 37, dark_green);
    (void)kode_fill_circle(c, 168, 116, 33, green);
    (void)kode_fill_circle(c, 176, 126, 20, belly);
    (void)kode_fill_circle(c, 141, 151, 11, dark_green);
    (void)kode_fill_circle(c, 194, 151, 11, dark_green);
    stroke(c, 134, 153, 151, 153, 5, light_green);
    stroke(c, 187, 153, 204, 153, 5, light_green);

    /* Neck, head, muzzle and two small horns. */
    stroke(c, 184, 101, 205, 91, 23, green);
    (void)kode_fill_circle(c, 209, 77, 29, green);
    (void)kode_fill_circle(c, 230, 91, 18, light_green);
    stroke(c, 192, 56, 184, 39, 7, gold);
    stroke(c, 184, 39, 198, 49, 5, gold);
    stroke(c, 220, 53, 230, 37, 7, gold);
    stroke(c, 230, 37, 235, 53, 5, gold);

    /* Face: expressive eyes, nostrils and a friendly smile. */
    (void)kode_fill_circle(c, 199, eye_y, 6, white);
    (void)kode_fill_circle(c, 222, eye_y, 6, white);
    (void)kode_fill_circle(c, 201, eye_y + 1, 3, ink);
    (void)kode_fill_circle(c, 224, eye_y + 1, 3, ink);
    (void)kode_fill_circle(c, 202, eye_y, 1, white);
    (void)kode_fill_circle(c, 225, eye_y, 1, white);
    (void)kode_fill_circle(c, 238, 89, 2, dark_green);
    (void)kode_fill_circle(c, 229, 102, 3, blush);
    (void)kode_fill_circle(c, 244, 99, 3, blush);
    stroke(c, 222, 108, 236, 111, 2, ink);

    /* Belly plates and little scale marks. */
    for (int i = 0; i < 3; ++i) {
        int y = 116 + i * 9;
        stroke(c, 163, y, 181, y + 2, 2, kode_rgb(224, 169, 91));
    }
    for (int i = 0; i < 3; ++i) {
        (void)kode_fill_circle(c, 151 + i * 13, 95 + (i % 2) * 7, 2, light_green);
    }

    /* Tiny fire puff when the dragon is happy and well fed. */
    if (hunger > 35 && happiness > 55) {
        (void)kode_fill_circle(c, 263, 104, 6, gold);
        (void)kode_fill_circle(c, 270, 99, 4, kode_rgb(255, 113, 67));
    }

    (void)kode_draw_text(c, 165, 174, "KODY  /  DRAGON", light_green, KODE_ALIGN_CENTER);
    (void)kode_draw_text(c, 14, 174, cleanliness < 20 ? "BATH TIME" : "LEVEL 01",
                         white, KODE_ALIGN_LEFT);
}

static const char *mood_name(int hunger, int happiness, int cleanliness) {
    if (hunger < 20) return "Kody is hungry";
    if (cleanliness < 20) return "Kody needs a wash";
    if (happiness < 20) return "Kody feels lonely";
    if (hunger > 75 && happiness > 75 && cleanliness > 75) return "Kody is delighted!";
    return "Kody is doing well";
}

static void render_stats(char *out, size_t cap, int hunger, int happiness,
                         int cleanliness, unsigned age, unsigned care) {
    (void)snprintf(out, cap,
        "HUNGER  %3d/100   HAPPY  %3d/100\n"
        "CLEAN   %3d/100   AGE    %3u min\n"
        "%s  |  CARE %u",
        hunger, happiness, cleanliness, age,
        mood_name(hunger, happiness, cleanliness), care);
}

void app_main(void) {
    kode_canvas_t dragon;
    kode_widget_t stats, feed, play, clean;
    int hunger = 82, happiness = 78, cleanliness = 86;
    unsigned age = 0U, care = 0U;
    uint32_t last_decay;
    char view[KODE_WIDGET_TEXT_MAX];

    kode_init();
    if (!kode_screen_new("Kody - Dragon Pet")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }

    dragon = kode_screen_add_canvas(CANVAS_W, CANVAS_H);
    stats = kode_screen_add_text("Meet Kody, your little dragon!");
    feed = kode_screen_add_button("Feed");
    play = kode_screen_add_button("Play");
    clean = kode_screen_add_button("Clean");
    if (!dragon || !stats || !feed || !play || !clean) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    (void)kode_widget_set_color(feed, kode_rgb(72, 177, 125));
    (void)kode_widget_set_color(play, kode_rgb(115, 105, 205));
    (void)kode_widget_set_color(clean, kode_rgb(65, 145, 190));
    draw_dragon(dragon, hunger, happiness, cleanliness);
    render_stats(view, sizeof(view), hunger, happiness, cleanliness, age, care);
    (void)kode_widget_set_text(stats, "%s", view);

    last_decay = kode_uptime_ms();
    for (;;) {
        uint32_t now = kode_uptime_ms();
        bool changed = false;
        if (kode_button_was_pressed("bottom")) kode_exit();

        if (kode_widget_was_pressed(feed)) {
            hunger = clamp_stat(hunger + 24);
            happiness = clamp_stat(happiness + 3);
            cleanliness = clamp_stat(cleanliness - 5);
            ++care;
            changed = true;
        }
        if (kode_widget_was_pressed(play)) {
            happiness = clamp_stat(happiness + 22);
            hunger = clamp_stat(hunger - 8);
            cleanliness = clamp_stat(cleanliness - 4);
            ++care;
            changed = true;
        }
        if (kode_widget_was_pressed(clean)) {
            cleanliness = clamp_stat(cleanliness + 28);
            happiness = clamp_stat(happiness + 4);
            ++care;
            changed = true;
        }

        if ((uint32_t)(now - last_decay) >= DECAY_MS) {
            unsigned elapsed = (now - last_decay) / DECAY_MS;
            hunger = clamp_stat(hunger - (int)(elapsed * 3U));
            happiness = clamp_stat(happiness - (int)(elapsed * 2U));
            cleanliness = clamp_stat(cleanliness - (int)(elapsed * 2U));
            age += elapsed;
            last_decay += elapsed * DECAY_MS;
            changed = true;
        }

        if (changed) {
            draw_dragon(dragon, hunger, happiness, cleanliness);
            render_stats(view, sizeof(view), hunger, happiness, cleanliness, age, care);
            (void)kode_widget_set_text(stats, "%s", view);
        }
        kode_delay(40U);
    }
}
