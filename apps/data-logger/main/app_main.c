/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "kode.h"
KODE_APP_MANIFEST("Data Logger","maker");
#define LOG_PATH "flash/sensor-log.csv"
void app_main(void){
 bool recording=false,header_ready=false;uint32_t samples=0;char line[160];
 kode_widget_t state,count,toggle;kode_init();
 if(!kode_screen_new("Data Logger")){kode_print("screen failed: %s",kode_error());kode_exit();}
 state=kode_screen_add_text("Stopped");count=kode_screen_add_value("0","Samples");toggle=kode_screen_add_button("Start / Stop");
 if(!state||!count||!toggle){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom")){if(recording)(void)kode_widget_set_text(state,"Stop recording first");else kode_exit();}
  if(kode_widget_was_pressed(toggle)||kode_button_was_pressed("top")){
   if(!recording){
    if(!header_ready){
     if(!kode_file_exists(LOG_PATH)){
      if(!kode_file_write(LOG_PATH,"uptime_ms,ax,ay,az,gx,gy,gz,temp_c\n")){(void)kode_widget_set_text(state,"Storage error: %.20s",kode_error());kode_delay(250U);continue;}
     }else if(!kode_file_append(LOG_PATH,"\n")){(void)kode_widget_set_text(state,"Storage error: %.20s",kode_error());kode_delay(250U);continue;}
     header_ready=true;
    }
    recording=true;(void)kode_widget_set_text(state,"Recording to sensor-log.csv");
   }else{recording=false;(void)kode_widget_set_text(state,"Stopped");}
  }
  if(recording){
   kode_imu_t imu=kode_imu_read();char sensor_error[64];(void)snprintf(sensor_error,sizeof(sensor_error),"%.62s",kode_error());
   if(isfinite(imu.accel_x)&&isfinite(imu.gyro_x)&&sensor_error[0]=='\0'){
    (void)snprintf(line,sizeof(line),"%lu,%.4f,%.4f,%.4f,%.3f,%.3f,%.3f,%.2f\n",(unsigned long)kode_uptime_ms(),imu.accel_x,imu.accel_y,imu.accel_z,imu.gyro_x,imu.gyro_y,imu.gyro_z,imu.temp);
    if(kode_file_append(LOG_PATH,line)){++samples;(void)kode_widget_set_text(count,"%lu",(unsigned long)samples);}
    else{recording=false;(void)kode_widget_set_text(state,"Write failed: %.20s",kode_error());}
   }else{recording=false;(void)kode_widget_set_text(state,"Sensor error: %.20s",sensor_error[0]?sensor_error:"invalid reading");}
  }
  kode_delay(1000U);
 }
}
