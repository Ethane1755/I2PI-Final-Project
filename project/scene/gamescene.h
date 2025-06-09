#ifndef GAMESCENE_H_INCLUDED
#define GAMESCENE_H_INCLUDED
#include "scene.h"
#include <stdbool.h>
/*
   [game scene object]
*/


typedef struct _GameScene
{
    ALLEGRO_BITMAP* background;
    int state;           // 0: 遊戲中, 1: 勝利等待, 2: 顯示win
    double timer;        // 用來累加經過的秒數
    ALLEGRO_BITMAP* win_img; // 勝利圖片
    bool win_show;

} GameScene;
Scene *New_GameScene(int label);
void game_scene_update(Scene *self);
void game_scene_draw(Scene *self);
void game_scene_destroy(Scene *self);

#endif
