/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include "kode.h"
KODE_APP_MANIFEST("Bluetooth Scanner","network");
void app_main(void){
 kode_widget_t status,results,scan;kode_ble_device_t devices[6];char text[300];
 kode_init();
 if(!kode_screen_new("Bluetooth Scanner")){kode_print("screen failed: %s",kode_error());kode_exit();}
 status=kode_screen_add_text("TOP: scan Bluetooth devices (3 s)");
 results=kode_screen_add_text("No scan yet");
 scan=kode_screen_add_button("Scan devices");
 if(!status||!results||!scan){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  if(kode_button_was_pressed("top")||kode_widget_was_pressed(scan)){
   (void)kode_widget_set_text(status,"Listening...");
   int n=kode_ble_scan(devices,6U,3000U);
   if(n<0){char err[64];snprintf(err,sizeof(err),"%.62s",kode_error());(void)kode_widget_set_text(results,"Scan failed: %s",err);}
   else{
    size_t used=0U;text[0]='\0';
    for(int i=0;i<n;i++){
     int wrote=snprintf(text+used,sizeof(text)-used,"%s%.18s\n%.17s %d dBm",
       i?"\n":"",devices[i].name[0]?devices[i].name:"(unnamed)",
       devices[i].address,(int)devices[i].rssi);
     if(wrote<0||(size_t)wrote>=sizeof(text)-used)break;
     used+=(size_t)wrote;
    }
    if(n==0)snprintf(text,sizeof(text),"No devices found");
    (void)kode_widget_set_text(results,"%s",text);
    (void)kode_widget_set_text(status,"%d devices heard",n);
   }
  }
  kode_delay(250U);
 }
}
