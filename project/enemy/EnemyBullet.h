#ifndef ENEMYBULLET_H_INCLUDED
#define ENEMYBULLET_H_INCLUDED

#include "../element/element.h"
#include "../shapes/Shape.h"
#include <allegro5/allegro.h>

typedef struct _EnemyBullet {
    float x, y;
    int width, height;
    float vx, vy; // x 和 y 方向的速度
    ALLEGRO_BITMAP *img;
    Shape *hitbox;
} EnemyBullet;

Elements *New_EnemyBullet(int label, float x, float y, float vx, float vy);
void EnemyBullet_update(Elements *self);
void EnemyBullet_interact(Elements *self);
void EnemyBullet_draw(Elements *self);
void EnemyBullet_destroy(Elements *self);
void _EnemyBullet_update_position(Elements *self, float dx, float dy);
void _EnemyBullet_interact_Character(Elements *self, Elements *tar);
void _EnemyBullet_interact_Tree(Elements *self, Elements *tar);
void _EnemyBullet_interact_Floor(Elements *self, Elements *tar);

#endif