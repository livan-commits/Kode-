/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <math.h>
#include "kode.h"
KODE_APP_MANIFEST("Level","maker");
#define RAD_TO_DEG 57.2957795f
void app_main(void){
 kode_widget_t pitch,roll,state;kode_init();
 if(!kode_screen_new("Digital Level")){kode_print("screen failed: %s",kode_error());kode_exit();}
 pitch=kode_screen_add_value("--","Pitch");roll=kode_screen_add_value("--","Roll");state=kode_screen_add_text("Hold Dot still");
 if(!pitch||!roll||!state){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  kode_imu_t imu=kode_imu_read();if(kode_button_was_pressed("bottom"))kode_exit();
  if(isfinite(imu.accel_x)&&isfinite(imu.accel_y)&&isfinite(imu.accel_z)){
   float p=atan2f(imu.accel_x,sqrtf(imu.accel_y*imu.accel_y+imu.accel_z*imu.accel_z))*RAD_TO_DEG;
   float r=atan2f(imu.accel_y,imu.accel_z)*RAD_TO_DEG;
   (void)kode_widget_set_text(pitch,"%+.1f deg",p);(void)kode_widget_set_text(roll,"%+.1f deg",r);
   (void)kode_widget_set_text(state,(fabsf(p)<1.5f&&fabsf(r)<1.5f)?"LEVEL":"Adjust position");
  }else (void)kode_widget_set_text(state,"IMU unavailable: %.20s",kode_error());
  kode_delay(200U);
 }
}
