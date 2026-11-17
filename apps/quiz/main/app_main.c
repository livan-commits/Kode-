/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include "kode.h"
KODE_APP_MANIFEST("Quiz", "play");
typedef struct {const char *q,*a[3];uint8_t correct;} question_t;
static const question_t qs[]={
 {"Capital of France?",{"Paris","Lyon","Nice"},0},
 {"2 + 5 = ?",{"6","7","8"},1},
 {"Red Planet?",{"Venus","Mars","Jupiter"},1},
 {"Largest ocean?",{"Atlantic","Indian","Pacific"},2},
 {"5 x 4 = ?",{"20","25","15"},0},
 {"Kode Dot MCU?",{"ESP32-P4","ESP32-C3","RP2040"},0},
 {"Water formula?",{"CO2","H2O","O2"},1},
 {"Days in a week?",{"5","6","7"},2}
};
#define N (sizeof(qs)/sizeof(qs[0]))
void app_main(void){
 kode_widget_t prompt,answers,status,next;uint8_t index=0,choice=0;unsigned score=0;bool answered=false,finished=false;
 kode_init();if(!kode_screen_new("Quiz")){kode_print("screen failed: %s",kode_error());kode_exit();}
 prompt=kode_screen_add_text("Press Start");answers=kode_screen_add_value("UP / RIGHT / DOWN","Choose answer");status=kode_screen_add_text("TOP confirm");next=kode_screen_add_button("Start / Next");
 if(!prompt||!answers||!status||!next){kode_print("widget creation failed");kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  if(kode_widget_was_pressed(next)){
   if(finished){index=0;score=0;choice=0;answered=false;finished=false;}
   else if(answered){index++;choice=0;answered=false;}
   if(index>=N)finished=true;
   if(finished){(void)kode_widget_set_text(prompt,"Quiz complete");(void)kode_widget_set_text(answers," ");(void)kode_widget_set_text(status,"Score: %u / %u",score,(unsigned)N);}
   else{(void)kode_widget_set_text(prompt,"%s",qs[index].q);(void)kode_widget_set_text(answers,"UP:%s\nRIGHT:%s\nDOWN:%s",qs[index].a[0],qs[index].a[1],qs[index].a[2]);(void)kode_widget_set_text(status,"Question %u/%u | TOP confirm",(unsigned)index+1U,(unsigned)N);}
  }
  if(!finished&&!answered){
   if(kode_button_was_pressed("up"))choice=0;
   if(kode_button_was_pressed("right"))choice=1;
   if(kode_button_was_pressed("down"))choice=2;
   if(kode_button_was_pressed("top")){answered=true;if(choice==qs[index].correct){score++;(void)kode_widget_set_text(status,"Correct! Score %u",score);(void)kode_haptic_play("success");}else (void)kode_widget_set_text(status,"Incorrect. Next question");}
   (void)kode_widget_set_text(answers,"UP:%s\nRIGHT:%s\nDOWN:%s\nSelected: %s",qs[index].a[0],qs[index].a[1],qs[index].a[2],qs[index].a[choice]);
  }
  kode_delay(40U);
 }
}
