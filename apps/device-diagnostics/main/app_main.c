/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "kode.h"
KODE_APP_MANIFEST("Device Diagnostics","security");
void app_main(void){
 kode_time_t now;kode_widget_t identity,system,clockw,radio;
 kode_init();
 if(!kode_screen_new("Device Diagnostics")){kode_print("screen failed: %s",kode_error());kode_exit();}
 identity=kode_screen_add_text("Reading device identity...");
 system=kode_screen_add_text("System status");
 clockw=kode_screen_add_text("Clock");
 radio=kode_screen_add_text("Connectivity");
 if(!identity||!system||!clockw||!radio){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 (void)kode_widget_set_text(identity,"ID %.28s",kode_device_id());
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  (void)kode_widget_set_text(system,"Uptime %lu s\nBattery %u%% / %lu mV",
   (unsigned long)(kode_uptime_ms()/1000U),(unsigned)kode_battery_level(),
   (unsigned long)kode_battery_voltage_mv());
  if(kode_time_local(&now))
   (void)kode_widget_set_text(clockw,"%04u-%02u-%02u %02u:%02u:%02u",
    (unsigned)now.year,(unsigned)now.month,(unsigned)now.day,
    (unsigned)now.hour,(unsigned)now.minute,(unsigned)now.second);
  else (void)kode_widget_set_text(clockw,"Clock not synchronized");
  (void)kode_widget_set_text(radio,"Wi-Fi %s\nUSB %s",
   kode_wifi_is_connected()?"connected":"disconnected",
   kode_power_is_usb_connected()?"connected":"disconnected");
  kode_delay(2000U);
 }
}
