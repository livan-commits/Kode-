/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "kode.h"
KODE_APP_MANIFEST("Settings","settings");
static unsigned clamp(unsigned v){return v>100U?100U:v;}
void app_main(void){
 unsigned brightness=60U,volume=0U;bool muted=false;const char *saved;
 kode_widget_t display,audio,help;kode_init();
 saved=kode_setting_get("app_brightness");
 if(saved&&saved[0]){unsigned v=(unsigned)strtoul(saved,NULL,10);if(v<=100U)brightness=v;}
 volume=kode_audio_get("volume");muted=kode_audio_get("mute")!=0U;
 if(!kode_screen_new("Settings")){kode_print("screen failed: %s",kode_error());kode_exit();}
 display=kode_screen_add_value("--","Brightness");
 audio=kode_screen_add_value("--","Volume");
 help=kode_screen_add_text("UP/DOWN brightness\nLEFT/RIGHT volume\nTOP mute, BOT home");
 if(!display||!audio||!help){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 (void)kode_display_set_brightness((uint8_t)brightness);
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  if(kode_button_was_pressed("up")){brightness=clamp(brightness+10U);(void)kode_display_set_brightness((uint8_t)brightness);(void)kode_setting_set("app_brightness",brightness==100U?"100":brightness==90U?"90":brightness==80U?"80":brightness==70U?"70":brightness==60U?"60":brightness==50U?"50":brightness==40U?"40":brightness==30U?"30":brightness==20U?"20":brightness==10U?"10":"0");}
  if(kode_button_was_pressed("down")){brightness=brightness>=10U?brightness-10U:0U;(void)kode_display_set_brightness((uint8_t)brightness);(void)kode_setting_set("app_brightness",brightness==100U?"100":brightness==90U?"90":brightness==80U?"80":brightness==70U?"70":brightness==60U?"60":brightness==50U?"50":brightness==40U?"40":brightness==30U?"30":brightness==20U?"20":brightness==10U?"10":"0");}
  if(kode_button_was_pressed("right")){volume=volume>=94U?100U:volume+6U;(void)kode_audio_set("volume",volume);}
  if(kode_button_was_pressed("left")){volume=volume>=6U?volume-6U:0U;(void)kode_audio_set("volume",volume);}
  if(kode_button_was_pressed("top")){muted=!muted;(void)kode_audio_set("mute",muted?1U:0U);}
  (void)kode_widget_set_text(display,"%u %%",brightness);
  (void)kode_widget_set_text(audio,"%u %% %s",volume,muted?"muted":"");
  kode_delay(120U);
 }
}
