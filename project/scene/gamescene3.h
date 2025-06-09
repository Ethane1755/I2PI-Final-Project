#ifndef GAMESCENE3_H_INCLUDED
#define GAMESCENE3_H_INCLUDED
#include "scene.h"
#include "../element/skill.h"
/*
   [game scene object]
*/


typedef struct _GameScene3
{
    ALLEGRO_BITMAP* background;
    int state;           // 0: 遊戲中, 1: 勝利等待, 2: 顯示win
    ALLEGRO_BITMAP* win_img; // 勝利圖片
    SkillSystem skill_sys;

} GameScene3;
Scene* New_GameScene3(int label);
void game_scene3_update(Scene* self);
void game_scene3_draw(Scene* self);
void game_scene3_destroy(Scene* self);

#endif
