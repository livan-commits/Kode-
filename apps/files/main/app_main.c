/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include "kode.h"
KODE_APP_MANIFEST("Files","core");
static const char *volumes[]={"flash","sd"};
void app_main(void){
 unsigned volume=0,selected=0;char folder[24]="flash";kode_widget_t path,listing,info;kode_init();
 if(!kode_screen_new("Files")){kode_print("screen failed: %s",kode_error());kode_exit();}
 path=kode_screen_add_text("flash/");listing=kode_screen_add_text("Loading...");info=kode_screen_add_text("UP/DOWN select; TOP switch volume");
 if(!path||!listing||!info){kode_print("widget creation failed: %s",kode_error());kode_exit();}
 for(;;){
  if(kode_button_was_pressed("bottom"))kode_exit();
  if(kode_button_was_pressed("top")){volume^=1U;selected=0;(void)snprintf(folder,sizeof(folder),"%s",volumes[volume]);}
  const kode_file_list_t *items=kode_file_list(folder);
  if(items){
   if(selected>=items->count)selected=items->count?items->count-1U:0U;
   (void)kode_widget_set_text(path,"%s/",folder);
   if(items->count==0U)(void)kode_widget_set_text(listing,"(empty)");
   else{
    char lines[180]="";size_t used=0;
    for(uint32_t i=0;i<items->count&&i<5U;i++){
     int n=snprintf(lines+used,sizeof(lines)-used,"%c %s%s\n",i==selected?'>':' ',items->entries[i].name,items->entries[i].is_folder?"/":"");
     if(n<0||(size_t)n>=sizeof(lines)-used)break;
     used+=(size_t)n;
    }
    (void)kode_widget_set_text(listing,"%s",lines);
   }
   (void)kode_widget_set_text(info,"%lu items%s",(unsigned long)items->count,items->more?" (more)":"");
  }else (void)kode_widget_set_text(listing,"Storage unavailable: %.20s",kode_error());
  if(kode_button_was_pressed("up")&&selected>0U)--selected;
  if(kode_button_was_pressed("down")&&items&&selected+1U<items->count)++selected;
  kode_delay(150U);
 }
}
