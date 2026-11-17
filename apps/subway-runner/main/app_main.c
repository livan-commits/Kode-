/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"
#define ROWS 8
#define LANES 3
#define PLAYER_ROW 7
#define TICK_MS 650U
enum { EMPTY=0, BARRIER=1, OVERHEAD=2, GAP=3 };
KODE_APP_MANIFEST("Subway Runner", "play");
static uint32_t next_random(uint32_t *state){*state=(*state*1664525U)+1013904223U;return *state;}
static void draw(char *out,size_t cap,const int obstacle[ROWS][LANES],const bool coin[ROWS][LANES],int lane,bool jumping,bool sliding){
 size_t p=0U;
 for(int r=0;r<ROWS;r++){for(int l=0;l<LANES;l++){char c='.';
  if(r==PLAYER_ROW&&l==lane)c=jumping?'^':(sliding?'v':'@');
  else if(obstacle[r][l]==BARRIER)c='X';else if(obstacle[r][l]==OVERHEAD)c='T';else if(obstacle[r][l]==GAP)c='_';else if(coin[r][l])c='$';
  if(p+1U<cap){out[p++]=c;}
  if(l<LANES-1&&p+1U<cap){out[p++]='|';}
 }if(p+1U<cap)out[p++]='\n';}out[p]='\0';
}
static void reset(int obstacle[ROWS][LANES],bool coin[ROWS][LANES],int *lane,int *jump,int *slide,unsigned *score,uint32_t *rng){
 for(int r=0;r<ROWS;r++)for(int l=0;l<LANES;l++){obstacle[r][l]=EMPTY;coin[r][l]=false;}
 *lane=1;*jump=0;*slide=0;*score=0U;*rng=0x4B4F4445U;
}
void app_main(void){
 int obstacle[ROWS][LANES]={{0}};bool coin[ROWS][LANES]={{false}};
 int lane=1,jump=0,slide=0;unsigned score=0U,best=0U;
 uint32_t rng=0x4B4F4445U,last=0U,now=0U;bool game_over=false;
 char frame[ROWS*(LANES*2)+1];kode_widget_t board,status,jump_button,slide_button;
 kode_init();if(!kode_screen_new("Subway Runner")){kode_print("screen failed: %s",kode_error());kode_exit();}
 board=kode_screen_add_text("Three lanes ahead...");status=kode_screen_add_text("LEFT/RIGHT lanes | TOP jump | DOWN slide");
 jump_button=kode_screen_add_button("Jump");slide_button=kode_screen_add_button("Slide");
 if(!board||!status||!jump_button||!slide_button){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 last=kode_uptime_ms();
 for(;;){now=kode_uptime_ms();if(kode_button_was_pressed("bottom"))kode_exit();
  if(game_over){if(kode_button_was_pressed("top")||kode_widget_was_pressed(jump_button)||kode_widget_was_pressed(slide_button)){reset(obstacle,coin,&lane,&jump,&slide,&score,&rng);game_over=false;last=now;}}
  else{
   if(kode_button_was_pressed("left"))lane=(lane+LANES-1)%LANES;
   if(kode_button_was_pressed("right"))lane=(lane+1)%LANES;
   if(kode_button_was_pressed("top")||kode_widget_was_pressed(jump_button)){jump=2;slide=0;}
   if(kode_button_was_pressed("down")||kode_widget_was_pressed(slide_button)){slide=2;jump=0;}
   if((uint32_t)(now-last)>=TICK_MS){
    for(int r=ROWS-1;r>0;r--)for(int l=0;l<LANES;l++){obstacle[r][l]=obstacle[r-1][l];coin[r][l]=coin[r-1][l];}
    for(int l=0;l<LANES;l++){obstacle[0][l]=EMPTY;coin[0][l]=false;}
    uint32_t roll=next_random(&rng);
    if((roll%100U)<72U){int target=(int)((roll>>8)%LANES);int type=(int)((roll>>16)%3U)+1;obstacle[0][target]=type;if((roll&0x100000U)!=0U)coin[0][(target+1)%LANES]=true;}
    else coin[0][(roll>>12)%LANES]=true;
    bool collected=coin[PLAYER_ROW][lane];if(collected){++score;coin[PLAYER_ROW][lane]=false;}
    int hit=obstacle[PLAYER_ROW][lane];if((hit==BARRIER||hit==GAP)&&jump==0)game_over=true;if(hit==OVERHEAD&&slide==0)game_over=true;
    if(game_over){if(score>best)best=score;}else{if(jump>0)--jump;if(slide>0)--slide;if(collected&&(score%5U)==0U)(void)kode_haptic_play("success");}
    last=now;
   }
  }
  draw(frame,sizeof(frame),obstacle,coin,lane,jump>0,slide>0);(void)kode_widget_set_text(board,"%s",frame);
  if(game_over)(void)kode_widget_set_text(status,"CRASH! Coins %u | Best %u | TOP restart",score,best);
  else (void)kode_widget_set_text(status,"Coins %u | Best %u | Lane %d",score,best,lane+1);
  kode_delay(30U);
 }
}
