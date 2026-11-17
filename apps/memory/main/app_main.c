/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Memory", "play");

static const char *direction(uint8_t key)
{
    static const char * const names[] = {"UP", "RIGHT", "DOWN", "LEFT"};
    return names[key & 3U];
}

static void show_sequence(kode_widget_t display, const uint8_t *sequence, uint8_t length)
{
    char text[64];
    size_t used = 0U;
    for (uint8_t i = 0U; i < length; ++i) {
        int written = snprintf(text + used, sizeof(text) - used, "%s%s",
                                i ? "  " : "", direction(sequence[i]));
        if (written < 0 || (size_t)written >= sizeof(text) - used) break;
        used += (size_t)written;
    }
    (void)kode_widget_set_text(display, "%s", text);
}

void app_main(void)
{
    kode_widget_t display, status, start;
    uint8_t sequence[12], level = 3U, input = 0U;
    uint32_t seed, show_until = 0U;
    bool showing = false, playing = false, finished = false;
    char status_text[48];

    kode_init();
    if (!kode_screen_new("Memory")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }
    display = kode_screen_add_value("---", "remember the sequence");
    status = kode_screen_add_text("Press Start");
    start = kode_screen_add_button("Start game");
    if (!display || !status || !start) {
        kode_print("widget creation failed");
        kode_exit();
    }

    seed = kode_uptime_ms() ^ 0x85ebca6bU;
    for (;;) {
        uint32_t now = kode_uptime_ms();
        int key = -1;

        if (kode_button_was_pressed("bottom")) kode_exit();
        if (kode_button_was_pressed("up")) key = 0;
        if (kode_button_was_pressed("right")) key = 1;
        if (kode_button_was_pressed("down")) key = 2;
        if (kode_button_was_pressed("left")) key = 3;

        if (kode_widget_was_pressed(start)) {
            level = 3U;
            input = 0U;
            finished = false;
            for (uint8_t i = 0U; i < level; ++i) {
                seed ^= seed << 13;
                seed ^= seed >> 17;
                seed ^= seed << 5;
                sequence[i] = (uint8_t)(seed & 3U);
            }
            show_sequence(display, sequence, level);
            (void)kode_widget_set_text(status, "Memorize level 1");
            showing = true;
            playing = false;
            show_until = now + 2500U;
        }

        if (showing && (int32_t)(now - show_until) >= 0) {
            showing = false;
            playing = true;
            input = 0U;
            (void)kode_widget_set_text(display, "Use the D-pad");
            (void)kode_widget_set_text(status, "Repeat the sequence");
        }

        if (playing && key >= 0) {
            if ((uint8_t)key != sequence[input]) {
                playing = false;
                finished = true;
                (void)kode_widget_set_text(status, "Wrong! Press Start");
                (void)kode_widget_set_text(display, "Game over");
            } else if (++input >= level) {
                playing = false;
                if (level >= 12U) {
                    finished = true;
                    (void)kode_widget_set_text(status, "You win! Press Start");
                    (void)kode_widget_set_text(display, "Perfect!");
                } else {
                    sequence[level] = (uint8_t)((seed = seed * 1664525U + 1013904223U) >> 30);
                    ++level;
                    input = 0U;
                    show_sequence(display, sequence, level);
                    (void)snprintf(status_text, sizeof(status_text), "Level %u - memorize", (unsigned)level - 2U);
                    (void)kode_widget_set_text(status, "%s", status_text);
                    showing = true;
                    show_until = now + 2500U;
                }
            } else {
                (void)kode_widget_set_text(status, "Correct, continue");
            }
        }
        (void)finished;
        kode_delay(30U);
    }
}
