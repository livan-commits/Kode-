/* SPDX-License-Identifier: GPL-3.0-or-later */
/**
 * @file app_main.c
 * @brief A compact four-operation calculator controlled by the Dot's buttons.
 */
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Calculator", "tools");

static void render(kode_widget_t a_widget, kode_widget_t b_widget, kode_widget_t op_widget,
                   kode_widget_t result_widget, int32_t a, int32_t b, uint32_t op)
{
    static const char * const operators[] = { "+", "-", "x", "/" };
    char result[32];

    (void)kode_widget_set_text(a_widget, "%ld", (long)a);
    (void)kode_widget_set_text(b_widget, "%ld", (long)b);
    (void)kode_widget_set_text(op_widget, "Operation: %s", operators[op]);
    if ((3U == op) && (0 == b)) {
        (void)snprintf(result, sizeof(result), "Cannot divide by zero");
    } else if (0U == op) {
        (void)snprintf(result, sizeof(result), "%ld", (long)(a + b));
    } else if (1U == op) {
        (void)snprintf(result, sizeof(result), "%ld", (long)(a - b));
    } else if (2U == op) {
        (void)snprintf(result, sizeof(result), "%ld", (long)(a * b));
    } else {
        (void)snprintf(result, sizeof(result), "%.2f", (double)a / (double)b);
    }
    (void)kode_widget_set_text(result_widget, "%s", result);
}

void app_main(void)
{
    kode_widget_t a_widget;
    kode_widget_t b_widget;
    kode_widget_t op_widget;
    kode_widget_t result_widget;
    kode_widget_t next_button;
    kode_widget_t reset_button;
    int32_t a = 0;
    int32_t b = 0;
    uint32_t op = 0U;

    kode_init();
    if (!kode_screen_new("Calculator")) { kode_print("screen failed: %s", kode_error()); kode_exit(); }
    a_widget = kode_screen_add_value("0", "A  (UP / DOWN)");
    b_widget = kode_screen_add_value("0", "B  (LEFT / RIGHT)");
    op_widget = kode_screen_add_text("Operation: +");
    result_widget = kode_screen_add_text("Result: 0");
    next_button = kode_screen_add_button("Next operation");
    reset_button = kode_screen_add_button("Reset values");
    if ((0U == a_widget) || (0U == b_widget) || (0U == op_widget) ||
        (0U == result_widget) || (0U == next_button) || (0U == reset_button)) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }
    render(a_widget, b_widget, op_widget, result_widget, a, b, op);

    for (;;) {
        if (kode_button_was_pressed("bottom")) { kode_exit(); }
        if (kode_button_was_pressed("up") && (999 > a)) { ++a; }
        if (kode_button_was_pressed("down") && (0 < a)) { --a; }
        if (kode_button_was_pressed("right") && (999 > b)) { ++b; }
        if (kode_button_was_pressed("left") && (0 < b)) { --b; }
        if (kode_button_was_pressed("top") || kode_widget_was_pressed(next_button)) { op = (op + 1U) % 4U; }
        if (kode_widget_was_pressed(reset_button)) { a = 0; b = 0; op = 0U; }
        render(a_widget, b_widget, op_widget, result_widget, a, b, op);
        kode_delay(100U);
    }
}
