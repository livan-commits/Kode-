/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <math.h>
#include <stdbool.h>
#include "kode.h"
KODE_APP_MANIFEST("Compass","tools");
static void show(kode_widget_t *h,kode_widget_t *s,kode_widget_t *c,kode_widget_t *x){
 if(!kode_screen_new("Compass")){kode_print("screen failed: %s",kode_error());kode_exit();}
 *h=kode_screen_add_value("--","Heading");*s=kode_screen_add_text("Calibrate before use");
 *c=kode_screen_add_button("Calibrate");*x=kode_screen_add_button("Clear calibration");
 if(!*h||!*s||!*c||!*x){kode_print("widget creation failed: %s",kode_error());kode_exit();}
}
void app_main(void){
 static const char *const dirs[]={"N","NE","E","SE","S","SW","W","NW"};
 kode_widget_t h,s,c,x;kode_init();show(&h,&s,&c,&x);
 for(;;){bool ready=kode_mag_is_calibrated();if(kode_button_was_pressed("bottom"))kode_exit();
  if(kode_widget_was_pressed(c)||kode_button_was_pressed("top")){if(kode_mag_calibrate())show(&h,&s,&c,&x);else (void)kode_widget_set_text(s,"Calibration failed: %.20s",kode_error());}
  if(kode_widget_was_pressed(x)||kode_button_was_pressed("left")){if(kode_mag_clear_calibration()){(void)kode_widget_set_text(h,"--");(void)kode_widget_set_text(s,"Calibration cleared");}else (void)kode_widget_set_text(s,"Clear failed: %.20s",kode_error());}
  if(ready){float d=kode_mag_heading();if(isfinite(d)){unsigned i=(unsigned)((d+22.5f)/45.0f)%8U;(void)kode_widget_set_text(h,"%.0f deg",d);(void)kode_widget_set_text(s,"Direction: %s",dirs[i]);}else (void)kode_widget_set_text(s,"Heading unavailable: %.20s",kode_error());}
  else (void)kode_widget_set_text(s,"Calibration required");
  kode_delay(250U);
 }
}
