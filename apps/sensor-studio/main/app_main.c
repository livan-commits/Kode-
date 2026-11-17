/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <math.h>
#include "kode.h"
KODE_APP_MANIFEST("Sensor Studio","maker");
void app_main(void){
 kode_widget_t motion,magnetic,status;kode_init();
 if(!kode_screen_new("Sensor Studio")){kode_print("screen failed: %s",kode_error());kode_exit();}
 motion=kode_screen_add_text("Reading IMU...");magnetic=kode_screen_add_text("Reading magnetometer...");status=kode_screen_add_text("Live sensor readings");
 if(!motion||!magnetic||!status){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  kode_imu_t imu=kode_imu_read();char imu_error[48];(void)snprintf(imu_error,sizeof(imu_error),"%.46s",kode_error());kode_mag_t mag=kode_mag_read();char mag_error[48];(void)snprintf(mag_error,sizeof(mag_error),"%.46s",kode_error());if(kode_button_was_pressed("bottom"))kode_exit();
  if(isfinite(imu.accel_x)&&isfinite(imu.gyro_x))
   (void)kode_widget_set_text(motion,"A %+.2f %+.2f %+.2f g\nG %+.1f %+.1f %+.1f d/s\nT %.1f C",imu.accel_x,imu.accel_y,imu.accel_z,imu.gyro_x,imu.gyro_y,imu.gyro_z,imu.temp);
  else (void)kode_widget_set_text(motion,"IMU unavailable: %.20s",imu_error[0]?imu_error:"invalid reading");
  if(isfinite(mag.x)&&isfinite(mag.y)&&isfinite(mag.z))
   (void)kode_widget_set_text(magnetic,"Mag %+.1f %+.1f %+.1f uT",mag.x,mag.y,mag.z);
  else (void)kode_widget_set_text(magnetic,"Magnetometer unavailable: %.16s",mag_error[0]?mag_error:"invalid reading");
  (void)kode_widget_set_text(status,"Live sensor readings");kode_delay(500U);
 }
}
