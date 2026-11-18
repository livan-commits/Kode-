/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Tamagotchi", "play");

#define DECAY_MS 60000U

static int clamp_stat(int value) {
    if (value < 0) return 0;
    if (value > 100) return 100;
    return value;
}

static const char *mood_name(int hunger, int happiness, int cleanliness) {
    if (hunger < 20) return "Hungry";
    if (cleanliness < 20) return "Needs a wash";
    if (happiness < 20) return "Lonely";
    if (hunger > 75 && happiness > 75 && cleanliness > 75) return "Very happy!";
    return "Doing well";
}

static void render_pet(char *out, size_t cap, int hunger, int happiness,
                       int cleanliness, unsigned age, unsigned care) {
    (void)snprintf(out, cap,
        "       /\\_/\\\n"
        "      ( o.o )\n"
        "       > ^ <\n"
        "       KODY\n\n"
        "Hunger      %3d/100\n"
        "Happiness   %3d/100\n"
        "Cleanliness %3d/100\n"
        "Age: %u min   Care: %u\n"
        "%s",
        hunger, happiness, cleanliness, age, care,
        mood_name(hunger, happiness, cleanliness));
}

void app_main(void) {
    kode_widget_t pet, feed, play, clean;
    int hunger = 82, happiness = 78, cleanliness = 86;
    unsigned age = 0U, care = 0U;
    uint32_t started, last_decay;
    char view[256];

    kode_init();
    if (!kode_screen_new("Tamagotchi")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }
    pet = kode_screen_add_text("Meet Kody!");
    feed = kode_screen_add_button("Feed");
    play = kode_screen_add_button("Play");
    clean = kode_screen_add_button("Clean");
    if (!pet || !feed || !play || !clean) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    started = kode_uptime_ms();
    last_decay = started;
    for (;;) {
        uint32_t now = kode_uptime_ms();
        if (kode_button_was_pressed("bottom")) kode_exit();

        if (kode_widget_was_pressed(feed)) {
            hunger = clamp_stat(hunger + 24);
            happiness = clamp_stat(happiness + 3);
            cleanliness = clamp_stat(cleanliness - 5);
            ++care;
        }
        if (kode_widget_was_pressed(play)) {
            happiness = clamp_stat(happiness + 22);
            hunger = clamp_stat(hunger - 8);
            cleanliness = clamp_stat(cleanliness - 4);
            ++care;
        }
        if (kode_widget_was_pressed(clean)) {
            cleanliness = clamp_stat(cleanliness + 28);
            happiness = clamp_stat(happiness + 4);
            ++care;
        }

        if ((uint32_t)(now - last_decay) >= DECAY_MS) {
            unsigned elapsed = (now - last_decay) / DECAY_MS;
            hunger = clamp_stat(hunger - (int)(elapsed * 3U));
            happiness = clamp_stat(happiness - (int)(elapsed * 2U));
            cleanliness = clamp_stat(cleanliness - (int)(elapsed * 2U));
            age += elapsed;
            last_decay += elapsed * DECAY_MS;
        }

        render_pet(view, sizeof(view), hunger, happiness, cleanliness, age, care);
        (void)kode_widget_set_text(pet, "%s", view);
        kode_delay(80U);
    }
}
