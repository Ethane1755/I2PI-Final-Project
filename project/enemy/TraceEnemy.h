#ifndef TRACEENEMY_H_INCLUDED
#define TRACEENEMY_H_INCLUDED

#include "../element/element.h"
#include "../shapes/Shape.h"
#include <allegro5/allegro.h>
#include <stdbool.h>

typedef enum {
    TE_STATE_MOVE,
    TE_STATE_HIT,
    TE_STATE_ATTACK,
    TE_STATE_DIE
} TraceEnemyState;

typedef struct _TraceEnemy {
    float x, y;
    int width, height;
    int speed;
    bool dir; 
    Shape *hitbox; 

    ALLEGRO_BITMAP *move_sheet;
    ALLEGRO_BITMAP *hit_sheet;
    ALLEGRO_BITMAP *attack_sheet;
    ALLEGRO_BITMAP *die_sheet;
    int move_frames;
    int hit_frames;
    int attack_frames;
    int die_frames;

    TraceEnemyState state;
    int frame;
    double last_frame_time;
    double frame_duration;

    int hp;
    int damage;
    double last_damage_time;
    double death_start_time;

    double last_shoot_time;
    double shoot_interval;
    float shoot_range;

} TraceEnemy;

Elements *New_TraceEnemy(int label, int x, int y);
void TraceEnemy_update(Elements *self);
void TraceEnemy_interact(Elements *self);
void TraceEnemy_draw(Elements *self);
void TraceEnemy_destroy(Elements *self);
void _TraceEnemy_update_position(Elements *self, float dx, float dy);
void TraceEnemy_take_damage(Elements *self, int damage, float kx, float ky);
void TraceEnemy_shoot(Elements *self, float target_x, float target_y);

#endif