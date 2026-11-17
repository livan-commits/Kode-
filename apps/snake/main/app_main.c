/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"
#define GRID_W 16
#define GRID_H 10
#define GRID_CELLS (GRID_W * GRID_H)
#define MAX_SNAKE GRID_CELLS
KODE_APP_MANIFEST("Snake", "play");
static uint32_t rng_state;
static uint32_t random_next(void) {
    rng_state ^= rng_state << 13; rng_state ^= rng_state >> 17; rng_state ^= rng_state << 5;
    return rng_state;
}
static bool occupied(const int16_t *xs, const int16_t *ys, uint16_t len, int x, int y) {
    for (uint16_t i = 0; i < len; ++i) if (xs[i] == x && ys[i] == y) return true;
    return false;
}
static void place_food(int16_t *fx, int16_t *fy, const int16_t *xs, const int16_t *ys, uint16_t len) {
    for (uint16_t tries = 0; tries < GRID_CELLS * 2U; ++tries) {
        int x = (int)(random_next() % GRID_W), y = (int)(random_next() % GRID_H);
        if (!occupied(xs, ys, len, x, y)) { *fx = (int16_t)x; *fy = (int16_t)y; return; }
    }
    *fx = -1; *fy = -1;
}
static void render_board(char *out, size_t cap, const int16_t *xs, const int16_t *ys,
                         uint16_t len, int fx, int fy) {
    size_t p = 0;
    for (int y = 0; y < GRID_H; ++y) {
        for (int x = 0; x < GRID_W; ++x) {
            char c = (x == fx && y == fy) ? '*' : ' ';
            for (uint16_t i = len; i > 0; --i)
                if (xs[i - 1U] == x && ys[i - 1U] == y) c = (i == 1U) ? '@' : 'o';
            if (p + 2U < cap) out[p++] = c;
        }
        if (p + 1U < cap) out[p++] = '\n';
    }
    out[p] = '\0';
}
void app_main(void) {
    kode_widget_t board, status, restart;
    int16_t sx[MAX_SNAKE], sy[MAX_SNAKE], fx, fy;
    uint16_t len = 3U, score = 0U;
    int dx = 1, dy = 0;
    bool running = true, over = false;
    uint32_t last_step, now;
    char view[(GRID_W + 1) * GRID_H + 1];
    kode_init();
    if (!kode_screen_new("Snake")) { kode_print("screen failed: %s", kode_error()); kode_exit(); }
    board = kode_screen_add_text("Loading");
    status = kode_screen_add_text("Score: 0 | TOP pause");
    restart = kode_screen_add_button("Restart");
    if (!board || !status || !restart) { kode_print("widget creation failed: %s", kode_error()); kode_exit(); }
    sx[0]=7; sy[0]=5; sx[1]=6; sy[1]=5; sx[2]=5; sy[2]=5;
    rng_state = kode_uptime_ms() ^ 0xa341316cU;
    place_food(&fx, &fy, sx, sy, len); last_step = kode_uptime_ms();
    for (;;) {
        now = kode_uptime_ms();
        if (kode_button_was_pressed("bottom")) kode_exit();
        if (kode_widget_was_pressed(restart)) {
            len=3U; score=0U; dx=1; dy=0; running=true; over=false;
            sx[0]=7; sy[0]=5; sx[1]=6; sy[1]=5; sx[2]=5; sy[2]=5;
            place_food(&fx,&fy,sx,sy,len); last_step=now;
        }
        if (kode_button_was_pressed("top") && !over) running = !running;
        if (kode_button_was_pressed("up") && dy != 1) { dx=0; dy=-1; }
        if (kode_button_was_pressed("down") && dy != -1) { dx=0; dy=1; }
        if (kode_button_was_pressed("left") && dx != 1) { dx=-1; dy=0; }
        if (kode_button_was_pressed("right") && dx != -1) { dx=1; dy=0; }
        if (running && !over && (uint32_t)(now-last_step) >= 220U) {
            int nx=sx[0]+dx, ny=sy[0]+dy;
            bool ate=(nx==fx && ny==fy);
            if (nx<0 || nx>=GRID_W || ny<0 || ny>=GRID_H ||
                occupied(sx,sy,(uint16_t)(len-(ate?0U:1U)),nx,ny)) { over=true; running=false; }
            else {
                uint16_t new_len=len;
                if (ate && len<MAX_SNAKE) ++new_len;
                for (uint16_t i=(uint16_t)(new_len-1U); i>0U; --i) { sx[i]=sx[i-1U]; sy[i]=sy[i-1U]; }
                sx[0]=(int16_t)nx; sy[0]=(int16_t)ny; len=new_len;
                if (ate) { ++score; place_food(&fx,&fy,sx,sy,len); }
            }
            last_step=now;
        }
        render_board(view,sizeof(view),sx,sy,len,fx,fy);
        (void)kode_widget_set_text(board,"%s",view);
        if (over) (void)kode_widget_set_text(status,"Game over! Score: %u | Restart",(unsigned)score);
        else (void)kode_widget_set_text(status,"Score: %u | %s",(unsigned)score,running?"TOP pause":"TOP resume");
        kode_delay(35U);
    }
}
