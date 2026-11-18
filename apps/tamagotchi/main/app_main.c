/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Kody 3D: a small, genuine software 3D renderer for the Kode Dot.
 * Geometry is transformed in 3D, perspective-projected, depth-sorted and
 * triangle-rasterized into the official Kode canvas. No 2D character sprite.
 */
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "kode.h"

KODE_APP_MANIFEST("Tamagotchi 3D", "play"); /* low-poly 3D smoke build */

#define PI_F 3.14159265358979323846f
#define FACE_MAX 480
#define SPHERE_RINGS 2
#define SPHERE_SLICES 6
#define CANVAS_W 330U
#define CANVAS_H 220U
#define CAMERA_Z 6.2f
#define FOCAL 300.0f

typedef struct { float x, y, z; } vec3_t;
typedef struct {
    vec3_t p[3];
    float depth;
    kode_color_t color;
} face_t;

static face_t faces[FACE_MAX];
static unsigned face_count;
static kode_canvas_t canvas;

static vec3_t v3(float x, float y, float z) { return (vec3_t){x,y,z}; }
static vec3_t add(vec3_t a, vec3_t b) { return v3(a.x+b.x,a.y+b.y,a.z+b.z); }
static vec3_t sub(vec3_t a, vec3_t b) { return v3(a.x-b.x,a.y-b.y,a.z-b.z); }
static vec3_t cross(vec3_t a, vec3_t b) {
    return v3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x);
}
static float dot(vec3_t a, vec3_t b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
static float length3(vec3_t a) { return sqrtf(dot(a,a)); }
static vec3_t normalize(vec3_t a) {
    float n=length3(a); return n>0.0001f ? v3(a.x/n,a.y/n,a.z/n) : v3(0,0,-1);
}
static vec3_t rotate_y(vec3_t p, float a) {
    float c=cosf(a),s=sinf(a); return v3(p.x*c+p.z*s,p.y,-p.x*s+p.z*c);
}
static vec3_t rotate_z(vec3_t p, float a) {
    float c=cosf(a),s=sinf(a); return v3(p.x*c-p.y*s,p.x*s+p.y*c,p.z);
}
static kode_color_t shade(kode_color_t rgb, float light) {
    unsigned r=(rgb>>16)&255U,g=(rgb>>8)&255U,b=rgb&255U;
    r=(unsigned)(r*light); g=(unsigned)(g*light); b=(unsigned)(b*light);
    if(r>255U) r=255U;
    if(g>255U) g=255U;
    if(b>255U) b=255U;
    return kode_rgb(r,g,b);
}
static vec3_t world_point(vec3_t p, float yaw, float bob) {
    p=rotate_y(p,yaw); p.y+=bob; return p;
}
static void push_face(vec3_t a, vec3_t b, vec3_t c, kode_color_t base) {
    if(face_count>=FACE_MAX) return;
    vec3_t n=normalize(cross(sub(b,a),sub(c,a)));
    vec3_t light=normalize(v3(-0.35f,0.65f,-0.9f));
    float diffuse=fabsf(dot(n,light));
    float intensity=0.43f+0.57f*diffuse;
    faces[face_count].p[0]=a; faces[face_count].p[1]=b; faces[face_count].p[2]=c;
    faces[face_count].depth=(a.z+b.z+c.z)/3.0f;
    faces[face_count].color=shade(base,intensity);
    ++face_count;
}
static void add_sphere(vec3_t center, vec3_t radius, kode_color_t color,
                       float yaw, float bob) {
    vec3_t grid[SPHERE_RINGS+1][SPHERE_SLICES];
    for(unsigned r=0;r<=SPHERE_RINGS;r++) {
        float phi=PI_F*(float)r/(float)SPHERE_RINGS;
        for(unsigned s=0;s<SPHERE_SLICES;s++) {
            float theta=2.0f*PI_F*(float)s/(float)SPHERE_SLICES;
            vec3_t p=v3(radius.x*sinf(phi)*cosf(theta),
                        radius.y*cosf(phi),
                        radius.z*sinf(phi)*sinf(theta));
            p=add(center,p); grid[r][s]=world_point(p,yaw,bob);
        }
    }
    for(unsigned r=0;r<SPHERE_RINGS;r++) for(unsigned s=0;s<SPHERE_SLICES;s++) {
        unsigned n=(s+1U)%SPHERE_SLICES;
        push_face(grid[r][s],grid[r+1U][s],grid[r+1U][n],color);
        push_face(grid[r][s],grid[r+1U][n],grid[r][n],color);
    }
}
static vec3_t wing_point(float x,float y,float z,int side,float flap,
                         float yaw,float bob) {
    vec3_t p=v3(x*(float)side,y,z);
    p=rotate_z(p,(float)side*flap);
    p.x+=(float)side*0.34f; p.y+=0.35f; p.z+=0.28f;
    return world_point(p,yaw,bob);
}
static void add_wings(float flap,float yaw,float bob) {
    const kode_color_t purple=kode_rgb(112,91,202);
    const kode_color_t lilac=kode_rgb(177,145,239);
    for(int side=-1;side<=1;side+=2) {
        vec3_t root=wing_point(0,0,0,side,flap,yaw,bob);
        vec3_t elbow=wing_point(0.62f,0.48f,0.02f,side,flap,yaw,bob);
        vec3_t tip=wing_point(1.25f,0.86f,0.05f,side,flap,yaw,bob);
        vec3_t lower=wing_point(0.92f,-0.42f,0.10f,side,flap,yaw,bob);
        vec3_t finger=wing_point(0.72f,0.12f,-0.08f,side,flap,yaw,bob);
        push_face(root,elbow,finger,purple);
        push_face(elbow,tip,finger,lilac);
        push_face(finger,tip,lower,purple);
        push_face(root,finger,lower,lilac);
        push_face(root,elbow,tip,purple);
        push_face(root,tip,lower,purple);
    }
}
static void add_tail(float yaw,float bob,float wag) {
    const kode_color_t green=kode_rgb(39,166,119);
    const kode_color_t tip=kode_rgb(255,185,66);
    for(unsigned i=0;i<3;i++) {
        float t=(float)i;
        vec3_t c=v3(-0.66f-0.27f*t,-0.22f-0.10f*t,
                    0.06f*sinf(wag+t*0.65f));
        add_sphere(c,v3(0.34f-0.065f*t,0.25f-0.05f*t,0.28f-0.05f*t),
                   i==2?tip:green,yaw,bob);
    }
}
static void build_dragon(float seconds,bool happy,bool blink) {
    face_count=0;
    float bob=0.055f*sinf(seconds*2.6f);
    float yaw=0.20f*sinf(seconds*0.55f);
    float flap=0.42f*sinf(seconds*5.0f);
    float wag=seconds*2.1f;
    const kode_color_t green=kode_rgb(49,190,137);
    const kode_color_t mint=kode_rgb(103,228,172);
    const kode_color_t dark=kode_rgb(28,125,94);
    const kode_color_t belly=kode_rgb(255,211,132);
    const kode_color_t gold=kode_rgb(250,180,64);
    const kode_color_t white=kode_rgb(255,255,244);
    const kode_color_t ink=kode_rgb(20,35,43);
    /* Back appendages first; the painter pass resolves their depth. */
    add_wings(flap,yaw,bob);
    add_tail(yaw,bob,wag);
    /* Four feet, body, belly, neck and expressive head. */
    add_sphere(v3(-0.48f,-0.63f,0.08f),v3(0.28f,0.20f,0.31f),dark,yaw,bob);
    add_sphere(v3(0.42f,-0.63f,0.08f),v3(0.28f,0.20f,0.31f),dark,yaw,bob);
    add_sphere(v3(-0.68f,-0.48f,-0.08f),v3(0.24f,0.17f,0.25f),green,yaw,bob);
    add_sphere(v3(0.62f,-0.48f,-0.08f),v3(0.24f,0.17f,0.25f),green,yaw,bob);
    add_sphere(v3(0.0f,-0.12f,0.0f),v3(0.83f,0.68f,0.59f),green,yaw,bob);
    add_sphere(v3(0.04f,-0.18f,-0.49f),v3(0.56f,0.48f,0.19f),belly,yaw,bob);
    add_sphere(v3(0.49f,0.43f,0.0f),v3(0.25f,0.43f,0.31f),mint,yaw,bob);
    add_sphere(v3(0.67f,0.74f,-0.01f),v3(0.54f,0.49f,0.47f),green,yaw,bob);
    add_sphere(v3(1.04f,0.49f,-0.12f),v3(0.38f,0.25f,0.34f),mint,yaw,bob);
    /* Horns, eyes, pupils and nostrils. */
    add_sphere(v3(0.39f,1.12f,0.02f),v3(0.12f,0.27f,0.12f),gold,yaw,bob);
    add_sphere(v3(0.82f,1.14f,0.02f),v3(0.12f,0.27f,0.12f),gold,yaw,bob);
    add_sphere(v3(0.48f,0.79f,-0.39f),v3(0.12f,0.15f,0.07f),white,yaw,bob);
    add_sphere(v3(0.83f,0.79f,-0.39f),v3(0.12f,0.15f,0.07f),white,yaw,bob);
    if(!blink) {
        add_sphere(v3(0.51f,0.78f,-0.455f),v3(0.055f,0.085f,0.035f),ink,yaw,bob);
        add_sphere(v3(0.86f,0.78f,-0.455f),v3(0.055f,0.085f,0.035f),ink,yaw,bob);
    }
    add_sphere(v3(1.15f,0.57f,-0.405f),v3(0.045f,0.035f,0.025f),dark,yaw,bob);
    if(happy) add_sphere(v3(1.37f,0.37f,-0.06f),v3(0.12f,0.12f,0.12f),
                         kode_rgb(255,126,64),yaw,bob);
}
static void project(vec3_t p,int *x,int *y) {
    float z=p.z+CAMERA_Z;
    if(z<1.0f)z=1.0f;
    *x=(int)(CANVAS_W*0.50f+(p.x*FOCAL/z));
    *y=(int)(CANVAS_H*0.53f-(p.y*FOCAL/z));
}
static void raster_triangle(const face_t *f) {
    int x[3],y[3];
    for(unsigned i=0;i<3;i++) project(f->p[i],&x[i],&y[i]);
    int miny=y[0],maxy=y[0];
    for(unsigned i=1;i<3;i++){if(y[i]<miny)miny=y[i];if(y[i]>maxy)maxy=y[i];}
    if(miny<0) miny=0;
    if(maxy>=(int)CANVAS_H) maxy=(int)CANVAS_H-1;
    for(int scan=miny;scan<=maxy;scan++) {
        int intersections[3],count=0;
        for(unsigned e=0;e<3;e++) {
            unsigned n=(e+1U)%3U;
            int y0=y[e],y1=y[n];
            if((scan>=y0 && scan<y1)||(scan>=y1 && scan<y0)) {
                float t=(float)(scan-y0)/(float)(y1-y0);
                intersections[count++]=(int)((float)x[e]+t*(float)(x[n]-x[e]));
            }
        }
        if(count>=2) {
            int left=intersections[0],right=intersections[1];
            if(left>right){int tmp=left;left=right;right=tmp;}
            if(right<0||left>=(int)CANVAS_W)continue;
            if(left<0) left=0;
            if(right>=(int)CANVAS_W) right=(int)CANVAS_W-1;
            (void)kode_draw_line(canvas,left,scan,right,scan,f->color);
        }
    }
}
static void render_frame(float seconds,bool happy,bool blink) {
    build_dragon(seconds,happy,blink);
    /* Painter's algorithm: distant triangles are painted before near ones. */
    for(unsigned i=1;i<face_count;i++) {
        face_t key=faces[i]; unsigned j=i;
        while(j>0 && faces[j-1].depth<key.depth){faces[j]=faces[j-1];--j;}
        faces[j]=key;
    }
    (void)kode_canvas_fill(canvas,kode_rgb(12,19,36));
    for(unsigned i=0;i<face_count;i++) raster_triangle(&faces[i]);
    /* Ground shadow and a few tiny stars, still part of the 3D scene framing. */
    (void)kode_fill_circle(canvas,165,190,3,kode_rgb(50,65,87));
    (void)kode_draw_text(canvas,165,204,"KODY  /  3D DRAGON PET",
                         kode_rgb(180,220,235),KODE_ALIGN_CENTER);
}
static int clamp_stat(int v){return v<0?0:(v>100?100:v);}
static const char *mood(int h,int p,int c) {
    if(h<20)return "Hungry";
    if(c<20)return "Needs a bath";
    if(p<20)return "Feeling lonely";
    if(h>75&&p>75&&c>75)return "Very happy!";
    return "Doing great";
}
void app_main(void) {
    kode_widget_t stats,feed,play,clean;
    int hunger=82,happiness=78,cleanliness=86;
    unsigned age=0,care=0,frames=0,fps=0;
    uint32_t last_decay,started,last_stats,last_fps;
    char text[ KODE_WIDGET_TEXT_MAX ];
    kode_init();
    if(!kode_screen_new("Kody - 3D Dragon")){kode_print("screen failed: %s",kode_error());kode_exit();}
    canvas=kode_screen_add_canvas(CANVAS_W,CANVAS_H);
    stats=kode_screen_add_text("Your 3D dragon is waking up...");
    feed=kode_screen_add_button("Feed");
    play=kode_screen_add_button("Play");
    clean=kode_screen_add_button("Clean");
    if(!canvas||!stats||!feed||!play||!clean){kode_print("UI failed: %s",kode_error());kode_exit();}
    (void)kode_widget_set_color(feed,kode_rgb(72,177,125));
    (void)kode_widget_set_color(play,kode_rgb(115,105,205));
    (void)kode_widget_set_color(clean,kode_rgb(65,145,190));
    started=last_decay=last_stats=last_fps=kode_uptime_ms();
    for(;;) {
        uint32_t now=kode_uptime_ms();
        float seconds=(float)(now-started)/1000.0f;
        bool changed=false;
        if(kode_button_was_pressed("bottom"))kode_exit();
        if(kode_widget_was_pressed(feed)){hunger=clamp_stat(hunger+24);happiness=clamp_stat(happiness+3);cleanliness=clamp_stat(cleanliness-5);++care;changed=true;}
        if(kode_widget_was_pressed(play)){happiness=clamp_stat(happiness+22);hunger=clamp_stat(hunger-8);cleanliness=clamp_stat(cleanliness-4);++care;changed=true;}
        if(kode_widget_was_pressed(clean)){cleanliness=clamp_stat(cleanliness+28);happiness=clamp_stat(happiness+4);++care;changed=true;}
        if((uint32_t)(now-last_decay)>=60000U){
            unsigned elapsed=(now-last_decay)/60000U;
            hunger=clamp_stat(hunger-(int)(elapsed*3U));
            happiness=clamp_stat(happiness-(int)(elapsed*2U));
            cleanliness=clamp_stat(cleanliness-(int)(elapsed*2U));
            age+=elapsed;last_decay+=elapsed*60000U;changed=true;
        }
        ++frames;
        if((uint32_t)(now-last_fps)>=1000U) {
            fps=frames; frames=0; last_fps=now; changed=true;
        }
        render_frame(seconds,hunger>35&&happiness>55,((unsigned)(seconds*2.0f)%9U)==0U);
        if(changed || (uint32_t)(now-last_stats)>=1000U) {
            (void)snprintf(text,sizeof(text),"HUNGER %d  HAPPY %d  CLEAN %d\nAGE %u min  CARE %u  FPS %u  %s",
                           hunger,happiness,cleanliness,age,care,fps,mood(hunger,happiness,cleanliness));
            (void)kode_widget_set_text(stats,"%s",text);
            last_stats=now;
        }
        kode_delay(100U);
    }
}
