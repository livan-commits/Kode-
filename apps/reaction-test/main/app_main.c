#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"
KODE_APP_MANIFEST("Reaction Test", "play");
void app_main(void) {
 kode_widget_t status,result,start; uint32_t seed,deadline=0,shown=0,best=0; bool waiting=false,ready=false; char s[32];
 kode_init();if(!kode_screen_new("Reaction Test")){kode_print("screen failed: %s",kode_error());kode_exit();}
 status=kode_screen_add_text("Start, then wait for GO");result=kode_screen_add_value("--","reaction time (ms)");start=kode_screen_add_button("Start");
 if(!status||!result||!start){kode_print("widget creation failed");kode_exit();}
 seed=kode_uptime_ms()^0xa341316cU;
 for(;;){uint32_t now=kode_uptime_ms();bool press=kode_button_was_pressed("top")||kode_button_was_pressed("up")||kode_button_was_pressed("down")||kode_button_was_pressed("left")||kode_button_was_pressed("right")||kode_widget_was_pressed(start);
 if(kode_button_was_pressed("bottom"))kode_exit();
 if(press&&waiting){waiting=false;kode_widget_set_text(status,"Too soon! Start again");kode_widget_set_text(result,"--");}
 else if(press&&ready){uint32_t t=now-shown;ready=false;snprintf(s,sizeof(s),"%lu",(unsigned long)t);kode_widget_set_text(result,"%s",s);if(!best||t<best)best=t;kode_widget_set_text(status,"Best: %lu ms",(unsigned long)best);}
 else if(press&&!waiting&&!ready){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;deadline=now+1500U+seed%3500U;waiting=true;kode_widget_set_text(status,"Wait for GO...");kode_widget_set_text(result,"--");}
 if(waiting&&(int32_t)(now-deadline)>=0){waiting=false;ready=true;shown=now;kode_widget_set_text(status,"GO! Press a direction");kode_haptic_play("success");}
 kode_delay(20U);}
}