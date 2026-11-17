#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"
KODE_APP_MANIFEST("Memory", "play");
static const char *dir(uint8_t n){static const char *v[]={"UP","RIGHT","DOWN","LEFT"};return v[n&3U];}
void app_main(void) {
 kode_widget_t display,status,start;uint8_t seq[12],level=3,input=0;uint32_t seed,until=0;bool showing=false,playing=false;char s[48];
 kode_init();if(!kode_screen_new("Memory")){kode_print("screen failed: %s",kode_error());kode_exit();}
 display=kode_screen_add_value("---","remember the sequence");status=kode_screen_add_text("Press Start");start=kode_screen_add_button("Start game");
 if(!display||!status||!start){kode_print("widget creation failed");kode_exit();}
 seed=kode_uptime_ms()^0x85ebca6bU;
 for(;;){uint32_t now=kode_uptime_ms();int key=-1;if(kode_button_was_pressed("bottom"))kode_exit();
 if(kode_button_was_pressed("up"))key=0;if(kode_button_was_pressed("right"))key=1;if(kode_button_was_pressed("down"))key=2;if(kode_button_was_pressed("left"))key=3;
 if(kode_widget_was_pressed(start)){level=3;input=0;playing=false;showing=true;for(uint8_t i=0;i<level;i++){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;seq[i]=(uint8_t)(seed&3U);}
 snprintf(s,sizeof(s),"%s %s %s",dir(seq[0]),dir(seq[1]),dir(seq[2]));kode_widget_set_text(display,"%s",s);kode_widget_set_text(status,"Memorize!");until=now+2500U;}
 if(showing&&(int32_t)(now-until)>=0){showing=false;playing=true;input=0;kode_widget_set_text(display,"Use the D-pad");kode_widget_set_text(status,"Repeat the sequence");}
 if(playing&&key>=0){if((uint8_t)key!=seq[input]){playing=false;kode_widget_set_text(status,"Wrong! Press Start");kode_widget_set_text(display,"Game over");}
 else if(++input>=level){playing=false;if(level<12)level++;kode_widget_set_text(status,"Level passed! Start next");kode_widget_set_text(display,"Well done!");}
 else kode_widget_set_text(status,"Correct, continue");}
 kode_delay(30U);}
}