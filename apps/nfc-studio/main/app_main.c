/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "kode.h"
KODE_APP_MANIFEST("NFC Studio","maker");
void app_main(void){
 kode_widget_t uid,kind,state;kode_init();
 if(!kode_screen_new("NFC Studio")){kode_print("screen failed: %s",kode_error());kode_exit();}
 uid=kode_screen_add_text("No tag detected");kind=kode_screen_add_text("Present an NFC tag");state=kode_screen_add_text("Read-only scanner");
 if(!uid||!kind||!state){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  kode_tag_t tag=kode_nfc_read_id(700U);
  if(tag.ok){(void)kode_widget_set_text(uid,"UID: %s",tag.id);(void)kode_widget_set_text(kind,"Type: %.24s",tag.name);(void)kode_widget_set_text(state,"Tag detected");}
  else (void)kode_widget_set_text(state,"Waiting for NFC tag...");
  kode_delay(150U);
 }
}
