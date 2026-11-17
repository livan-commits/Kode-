/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "kode.h"
KODE_APP_MANIFEST("LED Studio","maker");
static const char *names[]={"Red","Green","Blue","White","Yellow","Cyan","Magenta","Off"};
static const uint8_t rgb[][3]={{255,0,0},{0,255,0},{0,0,255},{255,255,255},{255,255,0},{0,255,255},{255,0,255},{0,0,0}};
void app_main(void){
 unsigned selected=0;kode_widget_t color,help;kode_init();
 if(!kode_screen_new("LED Studio")){kode_print("screen failed: %s",kode_error());kode_exit();}
 color=kode_screen_add_text("Red");help=kode_screen_add_text("UP/DOWN select, TOP apply");
 if(!color||!help){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom")){(void)kode_led_off();kode_exit();}
  if(kode_button_was_pressed("up"))selected=(selected+7U)%8U;
  if(kode_button_was_pressed("down"))selected=(selected+1U)%8U;
  (void)kode_widget_set_text(color,"%s",names[selected]);
  if(kode_button_was_pressed("top")){
   if(selected==7U)(void)kode_led_off();else (void)kode_led_set_rgb(rgb[selected][0],rgb[selected][1],rgb[selected][2]);
   (void)kode_widget_set_text(help,"Applied: %s",kode_error()[0]?"failed":"OK");
  }
  kode_delay(80U);
 }
}
