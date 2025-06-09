#ifndef GAMESCENE_H_INCLUDED
#define GAMESCENE_H_INCLUDED
#include "scene.h"
#include <stdbool.h>

#include "../element/skill.h"
/*
   [game scene object]
*/


typedef struct _GameScene
{
    ALLEGRO_BITMAP* background;
    int state;           // 0: 遊戲中, 1: 勝利等待, 2: 顯示win
    ALLEGRO_BITMAP* win_img; // 勝利圖片
    SkillSystem skill_sys;


} GameScene;
Scene *New_GameScene(int label);
void game_scene_update(Scene *self);
void game_scene_draw(Scene *self);
void game_scene_destroy(Scene *self);


#endif
