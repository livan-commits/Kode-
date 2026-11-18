/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Digital Clock", "life");

static const char *weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static void render_clock(kode_widget_t time_widget, kode_widget_t date_widget,
                         bool twelve_hour) {
    kode_time_t now;
    char time_text[32];
    char date_text[48];

    if (!kode_time_local(&now)) {
        (void)kode_widget_set_text(time_widget, "--:--:--");
        (void)kode_widget_set_text(date_widget, "Clock not set. Connect to time service.");
        return;
    }

    if (twelve_hour) {
        uint8_t hour = (uint8_t)(now.hour % 12U);
        if (hour == 0U) hour = 12U;
        (void)snprintf(time_text, sizeof(time_text), "%02u:%02u:%02u %s",
            (unsigned)hour, (unsigned)now.minute, (unsigned)now.second,
            now.hour >= 12U ? "PM" : "AM");
    } else {
        (void)snprintf(time_text, sizeof(time_text), "%02u:%02u:%02u",
            (unsigned)now.hour, (unsigned)now.minute, (unsigned)now.second);
    }

    (void)snprintf(date_text, sizeof(date_text), "%s, %02u %s %04u",
        now.weekday < 7U ? weekdays[now.weekday] : "Unknown",
        (unsigned)now.day,
        now.month >= 1U && now.month <= 12U ? months[now.month - 1U] : "???",
        (unsigned)now.year);
    (void)kode_widget_set_text(time_widget, "%s", time_text);
    (void)kode_widget_set_text(date_widget, "%s", date_text);
}

void app_main(void) {
    bool twelve_hour = false;
    kode_widget_t time_widget, date_widget, format_button;

    kode_init();
    if (!kode_screen_new("Digital Clock")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }

    time_widget = kode_screen_add_value("--:--:--", "");
    date_widget = kode_screen_add_text("Loading local date...");
    format_button = kode_screen_add_button("Switch to 12-hour");
    if (!time_widget || !date_widget || !format_button) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    for (;;) {
        if (kode_button_was_pressed("bottom")) kode_exit();
        if (kode_button_was_pressed("top") || kode_widget_was_pressed(format_button)) {
            twelve_hour = !twelve_hour;
            (void)kode_widget_set_text(format_button, "%s",
                twelve_hour ? "Switch to 24-hour" : "Switch to 12-hour");
        }
        render_clock(time_widget, date_widget, twelve_hour);
        kode_delay(500U);
    }
}
