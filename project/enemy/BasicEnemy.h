/* BasicEnemy.h */
#ifndef BASICENEMY_H_INCLUDED
#define BASICENEMY_H_INCLUDED

#include "../element/element.h"
#include "../shapes/Shape.h"
#include <allegro5/allegro.h>
#include <stdbool.h>

typedef enum {
    BE_STATE_RUN,
    BE_STATE_HIT,
    BE_STATE_ATTACK,
    BE_STATE_DEATH
} BasicEnemyState;

typedef struct _BasicEnemy {
    float x, y;
    int width, height;
    int speed;
    bool dir; 
    Shape *hitbox; 

    ALLEGRO_BITMAP *run_sheet;
    ALLEGRO_BITMAP *hit_sheet;
    ALLEGRO_BITMAP *attack_sheet;
    ALLEGRO_BITMAP *death_sheet;
    int run_frames;
    int hit_frames;
    int attack_frames;
    int death_frames;

    BasicEnemyState state;
    int frame;
    double last_frame_time;
    double frame_duration;

    int hp;
    int damage;
    double last_damage_time;
    double death_start_time;

} BasicEnemy;

Elements *New_BasicEnemy(int label, int x, int y);
void BasicEnemy_update(Elements *self);
void BasicEnemy_interact(Elements *self);
void BasicEnemy_draw(Elements *self);
void BasicEnemy_destroy(Elements *self);
void _BasicEnemy_update_position(Elements *self, float dx, float dy);
void BasicEnemy_take_damage(Elements *self, int damage, float kx, float ky);

#endif