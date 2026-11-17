/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"
#define FIELD_W 22
#define FIELD_H 12
#define PADDLE_H 3
KODE_APP_MANIFEST("Pong", "play");
static int clamp_int(int v,int lo,int hi){return v<lo?lo:(v>hi?hi:v);}
static void draw(char *out,size_t cap,int bx,int by,int py,int ay){
    size_t p=0;
    for(int y=0;y<FIELD_H;++y){
        for(int x=0;x<FIELD_W;++x){
            char c=' ';
            if(y==0||y==FIELD_H-1)c='-';
            else if(x==0||x==FIELD_W-1)c='|';
            else if(x==FIELD_W/2)c='.';
            if(x==1&&y>=py&&y<py+PADDLE_H)c='|';
            if(x==FIELD_W-2&&y>=ay&&y<ay+PADDLE_H)c='|';
            if(x==bx&&y==by)c='O';
            if(p+2U<cap)out[p++] = c;
        }
        if(p+1U<cap)out[p++]='\n';
    }
    out[p]='\0';
}
void app_main(void){
    kode_widget_t field,status,serve;
    int bx=FIELD_W/2,by=FIELD_H/2,vx=1,vy=1;
    int player_y=(FIELD_H-PADDLE_H)/2,ai_y=player_y;
    unsigned player_score=0U,ai_score=0U;
    bool running=false;
    uint32_t last=0U,now;
    char view[(FIELD_W+1)*FIELD_H+1];
    kode_init();
    if(!kode_screen_new("Pong")){kode_print("screen failed: %s",kode_error());kode_exit();}
    field=kode_screen_add_text("Press Serve");
    status=kode_screen_add_text("UP/DOWN move | TOP pause");
    serve=kode_screen_add_button("Serve / Restart");
    if(!field||!status||!serve){kode_print("widget creation failed: %s",kode_error());kode_exit();}
    for(;;){
        now=kode_uptime_ms();
        if(kode_button_was_pressed("bottom"))kode_exit();
        if(kode_button_was_pressed("up"))player_y=clamp_int(player_y-1,1,FIELD_H-PADDLE_H-1);
        if(kode_button_was_pressed("down"))player_y=clamp_int(player_y+1,1,FIELD_H-PADDLE_H-1);
        if(kode_button_was_pressed("top"))running=!running;
        if(kode_widget_was_pressed(serve)){
            bx=FIELD_W/2;by=FIELD_H/2;vx=((player_score+ai_score)%2U)?-1:1;
            vy=(player_score%2U)?-1:1;running=true;last=now;
        }
        if(running&&(uint32_t)(now-last)>=115U){
            int nx=bx+vx,ny=by+vy;
            if(ny<=1||ny>=FIELD_H-2){vy=-vy;ny=by+vy;}
            if(vx<0&&nx==1&&ny>=player_y&&ny<player_y+PADDLE_H){
                vx=1;vy=(ny==player_y)?-1:((ny==player_y+2)?1:vy);nx=bx+vx;
            }else if(vx>0&&nx==FIELD_W-2&&ny>=ai_y&&ny<ai_y+PADDLE_H){
                vx=-1;vy=(ny==ai_y)?-1:((ny==ai_y+2)?1:vy);nx=bx+vx;
            }
            bx=nx;by=ny;
            if(bx<=0){++ai_score;running=false;}
            if(bx>=FIELD_W-1){++player_score;running=false;}
            if(ai_y+1<by)++ai_y;else if(ai_y+1>by)--ai_y;
            ai_y=clamp_int(ai_y,1,FIELD_H-PADDLE_H-1);
            last=now;
        }
        draw(view,sizeof(view),bx,by,player_y,ai_y);
        (void)kode_widget_set_text(field,"%s",view);
        (void)kode_widget_set_text(status,"You %u - CPU %u | %s",player_score,ai_score,running?"TOP pause":"Serve to play");
        kode_delay(30U);
    }
}
