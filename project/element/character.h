#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED
#include <allegro5/allegro_audio.h>
#include "element.h"
#include "../shapes/Shape.h"
#include "../algif5/algif.h"
#include <stdbool.h>

/*
   [character object]
*/
typedef enum CharacterType
{
    IDLE = 0,
    MOVE
} CharacterType;
typedef struct _Character
{
    int x, y;
    int width, height;              // the width and height of image
    bool dir;                       // true: face to right, false: face to left
    int state;                      // the state of character
    ALGIF_ANIMATION *gif_status[3]; // gif for each state. 0: stop, 1: move, 2:attack
    ALLEGRO_SAMPLE_INSTANCE *atk_Sound;
    int anime;      // counting the time of animation
    int anime_time; // indicate how long the animation
    bool new_proj;
    Shape *hitbox; // the hitbox of object
    int hp;        // 玩家血量
} Character;
Elements *New_Character(int label);
void Character_update(Elements *self);
void Character_interact(Elements *self);
void Character_draw(Elements *self);
void Character_destory(Elements *self);
void _Character_update_position(Elements *self, int dx, int dy);
void Character_take_damage(Elements *self, int damage);

#endif
