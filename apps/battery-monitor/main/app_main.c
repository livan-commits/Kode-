/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include "kode.h"
KODE_APP_MANIFEST("Battery Monitor","security");
void app_main(void){
 kode_widget_t level,voltage,state;char err[64];kode_init();
 if(!kode_screen_new("Battery Monitor")){kode_print("screen failed: %s",kode_error());kode_exit();}
 level=kode_screen_add_value("--","Charge");voltage=kode_screen_add_value("--","Voltage");state=kode_screen_add_text("Reading battery gauge...");
 if(!level||!voltage||!state){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  uint8_t pct=kode_battery_level();snprintf(err,sizeof(err),"%.62s",kode_error());
  if(err[0]){(void)kode_widget_set_text(state,"Gauge unavailable: %s",err);kode_delay(2000U);continue;}
  uint32_t mv=kode_battery_voltage_mv();snprintf(err,sizeof(err),"%.62s",kode_error());
  if(err[0]){(void)kode_widget_set_text(state,"Voltage unavailable: %s",err);kode_delay(2000U);continue;}
  bool charging=kode_battery_is_charging();snprintf(err,sizeof(err),"%.62s",kode_error());
  if(err[0]){(void)kode_widget_set_text(state,"Charge status unavailable: %s",err);kode_delay(2000U);continue;}
  bool usb=kode_power_is_usb_connected();
  if(kode_button_was_pressed("bottom"))kode_exit();
  (void)kode_widget_set_text(level,"%u %%",(unsigned)pct);
  (void)kode_widget_set_text(voltage,"%lu mV",(unsigned long)mv);
  (void)kode_widget_set_text(state,"%s | USB %s",charging?"Charging":"Not charging",usb?"connected":"off");
  kode_delay(2000U);
 }
}
