/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "kode.h"

#define FIELD_W 20
#define MAX_LEVELS 10
#define BASE_LEFT 4
#define BASE_WIDTH 12
KODE_APP_MANIFEST("Tower Stack", "play");

static int clamp(int v,int lo,int hi){return v<lo?lo:(v>hi?hi:v);}

static void render(char *out,size_t cap,const int *left,const int *width,
                   int levels,int moving_x,int moving_w,bool game_over){
    size_t p=0U;
    for(int row=0;row<MAX_LEVELS+1;row++){
        for(int x=0;x<FIELD_W;x++){
            char c=' ';
            if(row==0&&!game_over&&x>=moving_x&&x<moving_x+moving_w)c='#';
            else if(row>0){
                int level=levels-row;
                if(level>=0&&x>=left[level]&&x<left[level]+width[level])c='#';
            }
            if(p+2U<cap)out[p++]=c;
        }
        if(p+1U<cap)out[p++]='\n';
    }
    out[p]='\0';
}

void app_main(void){
    kode_widget_t board,status,drop;
    int left[MAX_LEVELS],width[MAX_LEVELS],levels=1;
    int moving_x=0,direction=1,moving_w=BASE_WIDTH;
    unsigned perfect=0U;
    bool game_over=false;
    uint32_t last=0U,now=0U;
    char view[(FIELD_W+1)*(MAX_LEVELS+1)+1];
    kode_init();
    if(!kode_screen_new("Tower Stack")){kode_print("screen failed: %s",kode_error());kode_exit();}
    board=kode_screen_add_text("Ready to stack");
    status=kode_screen_add_text("LEFT/RIGHT move | TOP drop");
    drop=kode_screen_add_button("Drop block");
    if(!board||!status||!drop){kode_print("widget creation failed: %s",kode_error());kode_exit();}
    left[0]=BASE_LEFT;width[0]=BASE_WIDTH;last=kode_uptime_ms();
    for(;;){
        now=kode_uptime_ms();
        if(kode_button_was_pressed("bottom"))kode_exit();
        if(game_over&&(kode_button_was_pressed("left")||kode_widget_was_pressed(drop))){
            levels=1;left[0]=BASE_LEFT;width[0]=BASE_WIDTH;
            moving_w=BASE_WIDTH;moving_x=0;direction=1;perfect=0U;
            game_over=false;last=now;
        }else if(!game_over&&(kode_button_was_pressed("top")||kode_widget_was_pressed(drop))){
            int old_left=left[levels-1],old_right=old_left+width[levels-1];
            int new_left=moving_x>old_left?moving_x:old_left;
            int new_right=(moving_x+moving_w)<old_right?(moving_x+moving_w):old_right;
            int overlap=new_right-new_left;
            if(overlap<=0){game_over=true;(void)kode_haptic_play("failure");}
            else{
                if(overlap==moving_w&&new_left==moving_x)++perfect;else perfect=0U;
                left[levels]=new_left;width[levels]=overlap;++levels;
                moving_w=overlap;moving_x=0;direction=1;
                (void)kode_haptic_play((overlap==width[levels-2])?"success":"click");
                if(levels>MAX_LEVELS)levels=MAX_LEVELS;
            }
            last=now;
        }
        if(!game_over&&(uint32_t)(now-last)>=
           (uint32_t)clamp(105-(levels*5),48,105)){
            int max_x=FIELD_W-moving_w;
            moving_x+=direction;
            if(moving_x>=max_x){moving_x=max_x;direction=-1;}
            if(moving_x<=0){moving_x=0;direction=1;}
            last=now;
        }
        render(view,sizeof(view),left,width,levels,moving_x,moving_w,game_over);
        (void)kode_widget_set_text(board,"%s",view);
        if(game_over)(void)kode_widget_set_text(status,"Tower fell! Score %d | LEFT restart",levels-1);
        else (void)kode_widget_set_text(status,"Height %d | Perfect %u | TOP drop",levels-1,perfect);
        kode_delay(25U);
    }
}
