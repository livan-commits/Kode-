/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "kode.h"
KODE_APP_MANIFEST("IR Studio","maker");
void app_main(void){
 kode_widget_t protocol,address,command,state;kode_init();
 if(!kode_screen_new("IR Studio")){kode_print("screen failed: %s",kode_error());kode_exit();}
 protocol=kode_screen_add_text("Protocol: --");address=kode_screen_add_text("Address: --");command=kode_screen_add_text("Command: --");state=kode_screen_add_text("Point a remote at Dot");
 if(!protocol||!address||!command||!state){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  kode_ir_code_t frame=kode_ir_receive(600U);
  if(frame.ok){
   (void)kode_widget_set_text(protocol,"Protocol: %.12s",frame.protocol);
   (void)kode_widget_set_text(address,"Address: 0x%04lX",(unsigned long)frame.address);
   (void)kode_widget_set_text(command,"Command: 0x%04lX",(unsigned long)frame.command);
   (void)kode_widget_set_text(state,"Signal received");
  }else (void)kode_widget_set_text(state,"Waiting for IR signal...");
 }
}
