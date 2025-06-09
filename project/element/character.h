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
    IDLE = 0, IDLED,
    UP, DOWN, LEFT, RIGHT,
    DIE
} CharacterType;

typedef struct _Character
{
    int x, y;
    int width, height;              // the width and height of image
    bool dir;                       // true: face to right, false: face to left
    int state;                      // the state of character
    ALGIF_ANIMATION *gif_status[7];
    ALLEGRO_SAMPLE_INSTANCE *atk_Sound;
    int anime;
    int anime_time;
    bool new_proj;
    Shape *hitbox;
    int hp;
    int last_move_dir;
    float weapon_angle;
    struct _Projectile *last_proj;
    int max_hp;
    bool shield_active;
    float speed;
    bool multi_shot;
    int multishot_level;
    bool pending_multishot;         // <-- add this
    double last_multishot_time;     // <-- add this
    int multishot_shots_left;
} Character;

Elements *New_Character(int label);
void Character_update(Elements *self);
void Character_interact(Elements *self);
void Character_draw(Elements *self);
void Character_destory(Elements *self);
void _Character_update_position(Elements *self, int dx, int dy);
void Character_take_damage(Elements *self, int damage);

#endif
