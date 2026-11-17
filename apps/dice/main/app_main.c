#include <stdint.h>
#include <stdio.h>
#include "kode.h"
KODE_APP_MANIFEST("Dice", "play");
void app_main(void) {
 kode_widget_t face, info, roll; uint32_t seed, count=0; char s[16];
 kode_init(); if(!kode_screen_new("Dice")){kode_print("screen failed: %s",kode_error());kode_exit();}
 face=kode_screen_add_value("1","D6"); info=kode_screen_add_text("TOP or Roll to throw"); roll=kode_screen_add_button("Roll");
 if(!face||!info||!roll){kode_print("widget creation failed");kode_exit();}
 seed=kode_uptime_ms()^0x9e3779b9U;
 for(;;){if(kode_button_was_pressed("bottom"))kode_exit();
 if(kode_widget_was_pressed(roll)||kode_button_was_pressed("top")||kode_button_was_pressed("up")||kode_button_was_pressed("right")){
 seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;snprintf(s,sizeof(s),"%lu",(unsigned long)(seed%6U+1U));kode_widget_set_text(face,"%s",s);kode_widget_set_text(info,"Rolls: %lu",(unsigned long)++count);kode_haptic_play("click");}
 kode_delay(40U);}
}