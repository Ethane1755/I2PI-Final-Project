#ifndef TRACINGBULLET_H_INCLUDED
#define TRACINGBULLET_H_INCLUDED

#include "../element/element.h"
#include "../shapes/Shape.h"
#include <allegro5/allegro.h>

typedef struct _TracingBullet {
    float x, y;
    int width, height;
    float vx, vy; 
    ALLEGRO_BITMAP *img;
    Shape *hitbox;

    double trace_start_time;    
    double trace_duration;     
    float trace_strength;      
    bool is_tracing;           

    int frame;
    int total_frames;
    double last_frame_time;
    double frame_duration;
    
} TracingBullet;

Elements *New_TracingBullet(int label, float x, float y, float vx, float vy);
void TracingBullet_update(Elements *self);
void TracingBullet_interact(Elements *self);
void TracingBullet_draw(Elements *self);
void TracingBullet_destroy(Elements *self);
void _TracingBullet_update_position(Elements *self, float dx, float dy);
void _TracingBullet_interact_Character(Elements *self, Elements *tar);
void _TracingBullet_interact_Tree(Elements *self, Elements *tar);
void _TracingBullet_interact_Floor(Elements *self, Elements *tar);

#endif