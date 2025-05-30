/* BossEnemy.h */
#ifndef BOSSENEMY_H_INCLUDED
#define BOSSENEMY_H_INCLUDED

#include "../element/element.h"
#include "../shapes/Shape.h"
#include <allegro5/allegro.h>
#include <stdbool.h>

typedef enum {
    BOSS_BE_STATE_WALK,
    BOSS_BE_STATE_STEALTH_IN,
    BOSS_BE_STATE_STEALTH_OUT,
    BOSS_BE_STATE_CAST,
    BOSS_BE_STATE_SPELL,
    BOSS_BE_STATE_ATTACK,
    BOSS_BE_STATE_HURT,
    BOSS_BE_STATE_DIE
} BossEnemyState;

typedef struct _BossEnemy {
    float x, y;
    int width, height;
    bool dir;

    Shape *hitbox;

    // 動畫帧
    ALLEGRO_BITMAP *walk_frames[8];
    ALLEGRO_BITMAP *attack_frames[10];
    ALLEGRO_BITMAP *cast_frames[9];
    ALLEGRO_BITMAP *spell_frames[16];
    ALLEGRO_BITMAP *hurt_frames[3];
    ALLEGRO_BITMAP *death_frames[10];  // 用於潛行動畫
    ALLEGRO_BITMAP *die_bitmap;        

    int frame;
    double last_frame_time;
    double frame_duration;

    bool in_stealth;
    bool is_invisible; 
    double last_stealth_time;
    double stealth_interval;
    double stealth_duration;  
    double stealth_start_time; 

    float target_x, target_y;
    bool cast_completed;

    int total_attack_count;    
    double skill_delay;        
    bool pending_skill;       
    double skill_trigger_time;  

    int hp;
    int damage;
    double last_damage_time;
    double invincible_time;     

    BossEnemyState state;
} BossEnemy;

Elements *New_BossEnemy(int label, int x, int y);
void BossEnemy_update(Elements *self);
void BossEnemy_draw(Elements *self);
void BossEnemy_interact(Elements *self);
void BossEnemy_destroy(Elements *self);

#endif /* BOSSENEMY_H_INCLUDED */