/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "kode.h"
KODE_APP_MANIFEST("RFID Studio","maker");
void app_main(void){
 kode_widget_t uid,kind,state;kode_init();
 if(!kode_screen_new("RFID Studio")){kode_print("screen failed: %s",kode_error());kode_exit();}
 uid=kode_screen_add_text("No tag detected");kind=kode_screen_add_text("EM4100 reader");state=kode_screen_add_text("Read-only scanner");
 if(!uid||!kind||!state){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  kode_tag_t tag=kode_rfid_read_id(700U);
  if(tag.ok){(void)kode_widget_set_text(uid,"ID: %s",tag.id);(void)kode_widget_set_text(kind,"Type: %.24s",tag.name);(void)kode_widget_set_text(state,"Tag detected");}
  else (void)kode_widget_set_text(state,"Waiting for RFID tag...");
  kode_delay(150U);
 }
}
