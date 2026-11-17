/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Calendar", "life");

static int64_t days_from_civil(int year, unsigned month, unsigned day)
{
    int era = (year >= 0) ? year / 400 : (year - 399) / 400;
    unsigned yoe = (unsigned)(year - era * 400);
    unsigned doy = (153U * (month + (month > 2U ? (unsigned)-3 : 9U)) + 2U) / 5U + day - 1U;
    unsigned doe = yoe * 365U + yoe / 4U - yoe / 100U + doy;
    return (int64_t)era * 146097 + (int64_t)doe - 719468;
}

static void render_month(kode_widget_t month_widget, kode_widget_t grid_widget,
                         int year, unsigned month, int today_year,
                         unsigned today_month, unsigned today_day)
{
    static const char *const names[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    char title[32];
    char grid[192];
    size_t used;
    int64_t first;
    unsigned first_weekday, days, day, col;

    (void)snprintf(title, sizeof(title), "%s %d", names[month - 1U], year);
    (void)kode_widget_set_text(month_widget, "%s", title);
    (void)snprintf(grid, sizeof(grid), "Su  Mo  Tu  We  Th  Fr  Sa\n");
    used = 0U;
    while (grid[used] != '\0') ++used;
    first = days_from_civil(year, month, 1U);
    first_weekday = (unsigned)((first + 4) % 7);
    if ((int64_t)0 != first_weekday) {
        for (col = 0U; col < first_weekday; ++col) {
            int n = snprintf(grid + used, sizeof(grid) - used, "    ");
            if (n > 0) used += (size_t)n;
        }
    }
    if (month == 12U) days = (unsigned)(days_from_civil(year + 1, 1U, 1U) - first);
    else days = (unsigned)(days_from_civil(year, month + 1U, 1U) - first);
    for (day = 1U; day <= days; ++day) {
        int n = snprintf(grid + used, sizeof(grid) - used,
                         (year == today_year && month == today_month && day == today_day) ? "%2u* " : "%2u  ", day);
        if (n > 0) used += (size_t)n;
        if (((first_weekday + day) % 7U) == 0U && day != days) {
            if (used + 2U < sizeof(grid)) { grid[used++] = '\n'; grid[used] = '\0'; }
        }
    }
    (void)kode_widget_set_text(grid_widget, "%s", grid);
}

void app_main(void)
{
    kode_time_t now;
    int year = 2026, today_year = 0;
    unsigned month = 1U, today_month = 0U, today_day = 0U;
    bool clock_ready;
    kode_widget_t month_widget, grid_widget, previous, next, today;
    kode_init();
    clock_ready = kode_time_local(&now);
    if (clock_ready) {
        year = (int)now.year; month = now.month;
        today_year = (int)now.year; today_month = now.month; today_day = now.day;
    }
    if (!kode_screen_new("Calendar")) { kode_print("screen failed: %s", kode_error()); kode_exit(); }
    month_widget = kode_screen_add_value(clock_ready ? "Current month" : "Clock not set", "Month");
    grid_widget = kode_screen_add_text(clock_ready ? "Su  Mo  Tu  We  Th  Fr  Sa" : "Set the KodeOS clock to use Calendar");
    previous = kode_screen_add_button("Previous month");
    next = kode_screen_add_button("Next month");
    today = kode_screen_add_button("Today");
    if (!month_widget || !grid_widget || !previous || !next || !today) {
        kode_print("widget creation failed: %s", kode_error()); kode_exit();
    }
    if (clock_ready) render_month(month_widget, grid_widget, year, month, today_year, today_month, today_day);

    for (;;) {
        bool changed = false;
        if (kode_button_was_pressed("bottom")) kode_exit();
        if (kode_widget_was_pressed(previous) || kode_button_was_pressed("left")) {
            if (month == 1U) { month = 12U; --year; } else { --month; }
            changed = true;
        }
        if (kode_widget_was_pressed(next) || kode_button_was_pressed("right")) {
            if (month == 12U) { month = 1U; ++year; } else { ++month; }
            changed = true;
        }
        if (kode_widget_was_pressed(today) || kode_button_was_pressed("top")) {
            if (clock_ready) { year = today_year; month = today_month; changed = true; }
        }
        if (changed && clock_ready) render_month(month_widget, grid_widget, year, month, today_year, today_month, today_day);
        kode_delay(50U);
    }
}
