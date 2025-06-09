#ifndef GAMESCENE2_H_INCLUDED
#define GAMESCENE2_H_INCLUDED
#include "scene.h"

/*
   [game scene object]
*/


typedef struct _GameScene2
{
     ALLEGRO_BITMAP* background;
        int state;           // 0: 遊戲中, 1: 勝利等待, 2: 顯示win
        double timer;        // 用來累加經過的秒數
        ALLEGRO_BITMAP* win_img; // 勝利圖片

} GameScene2;
Scene* New_GameScene2(int label);
void game_scene2_update(Scene* self);
void game_scene2_draw(Scene* self);
void game_scene2_destroy(Scene* self);

#endif
