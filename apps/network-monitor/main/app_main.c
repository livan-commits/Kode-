/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include "kode.h"
KODE_APP_MANIFEST("Network Monitor","network");
void app_main(void){
 kode_widget_t state,result,action;kode_init();
 if(!kode_screen_new("Network Monitor")){kode_print("screen failed: %s",kode_error());kode_exit();}
 state=kode_screen_add_text("Checking Wi-Fi...");
 result=kode_screen_add_text("TOP: ping 1.1.1.1");
 action=kode_screen_add_button("Ping gateway");
 if(!state||!result||!action){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  bool connected=kode_wifi_is_connected();
  (void)kode_widget_set_text(state,"Wi-Fi: %s",connected?"connected":"not connected");
  if(kode_button_was_pressed("top")||kode_widget_was_pressed(action)){
   if(!connected)(void)kode_widget_set_text(result,"Connect Wi-Fi in KodeOS Settings first");
   else{
    kode_net_ping_t p=kode_net_ping("1.1.1.1",2U);
    if(!p.ok){char err[48];snprintf(err,sizeof(err),"%.46s",kode_error());(void)kode_widget_set_text(result,"Ping error: %s",err);}
    else if(!p.received)(void)kode_widget_set_text(result,"No echo reply (network may block ICMP)");
    else (void)kode_widget_set_text(result,"%lu/%lu replies, avg %lu ms",
      (unsigned long)p.received,(unsigned long)p.sent,(unsigned long)p.avg_ms);
   }
  }
  kode_delay(300U);
 }
}
