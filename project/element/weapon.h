#ifndef WEAPON_H
#define WEAPON_H

#include "../scene/sceneManager.h"
#include "character.h"
#include "../algif5/algif.h"

void Weapon_draw(int x, int y);
void Weapon_draw_rotated(int x, int y, float angle);
void Character_fire_projectile(Character *chara, int state);

#endif