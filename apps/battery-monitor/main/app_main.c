/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "kode.h"
KODE_APP_MANIFEST("Battery Monitor","security");
void app_main(void){
 kode_widget_t level,voltage,state;kode_init();
 if(!kode_screen_new("Battery Monitor")){kode_print("screen failed: %s",kode_error());kode_exit();}
 level=kode_screen_add_value("--","Charge");voltage=kode_screen_add_value("--","Voltage");state=kode_screen_add_text("Reading battery gauge...");
 if(!level||!voltage||!state){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  uint8_t pct=kode_battery_level();uint32_t mv=kode_battery_voltage_mv();
  bool charging=kode_battery_is_charging();bool usb=kode_power_is_usb_connected();
  if(kode_button_was_pressed("bottom"))kode_exit();
  if(kode_error()[0]=='\0'){
   (void)kode_widget_set_text(level,"%u %%",(unsigned)pct);
   (void)kode_widget_set_text(voltage,"%lu mV",(unsigned long)mv);
   (void)kode_widget_set_text(state,"%s | USB %s",charging?"Charging":"Not charging",usb?"connected":"off");
  }else (void)kode_widget_set_text(state,"Gauge unavailable: %.22s",kode_error());
  kode_delay(2000U);
 }
}
