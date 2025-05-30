#ifndef BULLETENEMY_H_INCLUDED
#define BULLETENEMY_H_INCLUDED

#include "../element/element.h"
#include "../shapes/Shape.h"
#include <allegro5/allegro.h>
#include <stdbool.h>

typedef enum {
    BLE_STATE_RUN,
    BLE_STATE_HIT,
    BLE_STATE_ATTACK,
    BLE_STATE_DEATH
} BulletEnemyState;

typedef struct _BulletEnemy {
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

    BulletEnemyState state;
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

} BulletEnemy;

Elements *New_BulletEnemy(int label, int x, int y);
void BulletEnemy_update(Elements *self);
void BulletEnemy_interact(Elements *self);
void BulletEnemy_draw(Elements *self);
void BulletEnemy_destroy(Elements *self);
void _BulletEnemy_update_position(Elements *self, float dx, float dy);
void BulletEnemy_take_damage(Elements *self, int damage, float kx, float ky);
void BulletEnemy_shoot(Elements *self, float target_x, float target_y);

#endif