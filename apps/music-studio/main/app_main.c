/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "kode.h"
KODE_APP_MANIFEST("Music Studio","play");
static const uint16_t hz[]={440,523,659,784,880,1000,1200};
void app_main(void){
 unsigned selected=3;kode_widget_t tone,state,play;kode_init();
 if(!kode_screen_new("Music Studio")){kode_print("screen failed: %s",kode_error());kode_exit();}
 tone=kode_screen_add_value("440 Hz","Tone");state=kode_screen_add_text("UP/DOWN frequency");play=kode_screen_add_button("Play tone");
 if(!tone||!state||!play){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  if(kode_button_was_pressed("up"))selected=(selected+6U)%7U;
  if(kode_button_was_pressed("down"))selected=(selected+1U)%7U;
  (void)kode_widget_set_text(tone,"%u Hz",(unsigned)hz[selected]);
  if(kode_button_was_pressed("top")||kode_widget_was_pressed(play)){
   bool ok=kode_audio_tone(hz[selected],250U);
   if(ok)(void)kode_widget_set_text(state,"Tone played");
   else (void)kode_widget_set_text(state,"Audio unavailable: %.18s",kode_error());
  }
  kode_delay(80U);
 }
}
