/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include "kode.h"
KODE_APP_MANIFEST("Wi-Fi Analyzer","network");
void app_main(void){
 kode_widget_t status,results,scan;kode_wifi_network_t nets[7];char text[300];
 kode_init();
 if(!kode_screen_new("Wi-Fi Analyzer")){kode_print("screen failed: %s",kode_error());kode_exit();}
 status=kode_screen_add_text("TOP: scan nearby Wi-Fi (about 38 s)");
 results=kode_screen_add_text("No scan yet");
 scan=kode_screen_add_button("Scan networks");
 if(!status||!results||!scan){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  if(kode_button_was_pressed("top")||kode_widget_was_pressed(scan)){
   (void)kode_widget_set_text(status,"Scanning... please wait");
   int n=kode_wifi_scan(nets,7U);
   if(n<0){char err[64];snprintf(err,sizeof(err),"%.62s",kode_error());(void)kode_widget_set_text(results,"Scan failed: %s",err);(void)kode_widget_set_text(status,"Try again later if radio was busy");}
   else{
    size_t used=0U;text[0]='\0';
    for(int i=0;i<n;i++){
     int wrote=snprintf(text+used,sizeof(text)-used,"%s%.20s %d dBm ch%u %s",
       i?"\n":"",nets[i].ssid[0]?nets[i].ssid:"(hidden)",(int)nets[i].rssi,
       (unsigned)nets[i].channel,nets[i].open?"open":"secured");
     if(wrote<0||(size_t)wrote>=sizeof(text)-used)break;used+=(size_t)wrote;
    }
    if(n==0)snprintf(text,sizeof(text),"No networks found");
    (void)kode_widget_set_text(results,"%s",text);
    (void)kode_widget_set_text(status,"%d networks found",n);
   }
  }
  kode_delay(250U);
 }
}
