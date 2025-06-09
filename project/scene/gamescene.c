#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_primitives.h> //
#include <allegro5/allegro_acodec.h> //
#include <allegro5/allegro_font.h> //
#include <allegro5/allegro_ttf.h> //
#include "gamescene.h"
#include "../element/element.h"
#include "../element/character.h"
#include "../element/floor.h"
#include "../element/teleport.h"
#include "../element/tree.h"
#include "../element/projectile.h"
#include "../element/ball.h"
#include "../enemy/BasicEnemy.h"
#include "../enemy/BulletEnemy.h"
#include "../enemy/TraceEnemy.h"
#include "../enemy/TracingBullet.h"
#include "../enemy/BossEnemy.h"
#include "../element/projectile.h"
#include "../element/elementLabel.h"
#include "scene.h"
#include "../element/skill.h"

/*
   [GameScene function]
*/
// Character character; //character



Scene* New_GameScene(int label)
{
    //printf("label=%d\n", label);
    GameScene* pDerivedObj = (GameScene*)malloc(sizeof(GameScene));
    Scene* pObj = New_Scene(label);
    // setting derived object member
    pDerivedObj->background = al_load_bitmap("assets/image/firstmap.png");
    SkillSystem_init(&pDerivedObj->skill_sys);
    pObj->pDerivedObj = pDerivedObj;
    // register element
    //_Register_elements(pObj, New_Floor(Floor_L));
    _Register_elements(pObj, New_Teleport(Teleport_L));
    //_Register_elements(pObj, New_Tree(Tree_L));
    _Register_elements(pObj, New_Character(Character_L));
    _Register_elements(pObj, New_Ball(Ball_L));
    _Register_elements(pObj, New_BasicEnemy(BasicEnemy_L, 100, 100));
    _Register_elements(pObj, New_BulletEnemy(BulletEnemy_L, 200, 200));
    _Register_elements(pObj, New_TraceEnemy(TraceEnemy_L, 300, 300));
    _Register_elements(pObj, New_BossEnemy(BossEnemy_L, 200, 200));
    // setting derived object function
    pObj->Update = game_scene_update;
    pObj->Draw = game_scene_draw;
    pObj->Destroy = game_scene_destroy;
    return pObj;
}

void game_scene_update(Scene* self)
{
    GameScene* gs = (GameScene*)self->pDerivedObj;
    ElementVec allEle = _Get_all_elements(self);

    // 先找到 player
    Elements* player_ele = NULL;
    for (int i = 0; i < allEle.len; i++) {
        if (allEle.arr[i]->label == Character_L) {
            player_ele = allEle.arr[i];
            break;
        }
    }
    if (player_ele) {
        Character* player = (Character*)(player_ele->pDerivedObj);
        SkillSystem_update(&gs->skill_sys, player);
    }

    // 技能選單開啟時，暫停所有遊戲行為
    if (gs->skill_sys.selecting) {
        return;
    }

    // update every element
    for (int i = 0; i < allEle.len; i++)
    {
        Elements* ele = allEle.arr[i];
        ele->Update(ele);
    }
    // run interact for every element
    for (int i = 0; i < allEle.len; i++)
    {
        Elements* ele = allEle.arr[i];
        ele->Interact(ele);
    }
    // remove element
    for (int i = 0; i < allEle.len; i++)
    {
        Elements* ele = allEle.arr[i];
        if (ele->dele)
            _Remove_elements(self, ele);
    }

    // Do NOT redeclare gs here!

    ElementVec allEnemies = _Get_all_enemies(self);
    printf("Enemy count: %d\n", allEnemies.len);

    if (allEnemies.len == 0) {
        //gs->timer = 0;
        
        ElementVec allEle = _Get_all_elements(self);
        for (int i = 0; i < allEle.len; i++) {
            Elements* ele = allEle.arr[i];
            ele->Destroy(ele);
        }
        if (gs->background) {
            al_destroy_bitmap(gs->background);
            gs->background = NULL;
        }
        if (!gs->win_img) {
            gs->win_img = al_load_bitmap("assets/image/win.png");
            gs->win_show = true;
            printf("gs->win_show=true!\n");
        }
        //gs->timer += 1.0 / 60.0;
        if (gs->win_show) {
            if (key_state[ALLEGRO_KEY_ENTER]) {
                //gs->timer = 0;
                // 這裡可以切換到下一個場景
                printf("Set window to %d (GameScene2_L)\n", window);
                self->scene_end = true;
                window = 2;
                //printf("Set window to %d (GameScene2_L)\n", window);
                return;
            }
        }        
    }
}

void game_scene_draw(Scene* self)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));
    GameScene* gs = ((GameScene*)(self->pDerivedObj));

    
    // 只顯示 win 畫面
    if (gs->win_img){
        al_draw_bitmap(gs->win_img, (WIDTH - al_get_bitmap_width(gs->win_img)) / 2, (HEIGHT - al_get_bitmap_height(gs->win_img)) / 2, 0);
        return;
    }
        

    // 取得角色座標
    ElementVec allEle = _Get_all_elements(self);
    int chara_x = 0;
    int chara_y = 0;
    for (int i = 0; i < allEle.len; i++) {
        Elements* ele = allEle.arr[i];
        if (ele->label == Character_L) {
            Character* chara = (Character*)(ele->pDerivedObj);
            chara_x = chara->x;
            chara_y = chara->y;
            break;
        }
    }

    //printf("Character position: (%d, %d)\n", chara_x, chara_y);

    // 計算 Camera
    Point Camera;
    Camera.x = 0; // x 固定不動
    Camera.y = chara_y;

    if (Camera.y < 0)
        Camera.y = 0;
    if (Camera.y > HEIGHT)
        Camera.y = HEIGHT;
    //printf("Camera.y=%lf\n", Camera.y);
    // 背景根據 Camera 偏移
    // if (chara_y < 200)
    //     Camera.y = 0;
    // else if (chara_y < 400)
    //     Camera.y = 250;
    // else
    //     Camera.y = 500;

    // // 限制 Camera.y 不超過地圖邊界（可選）
    // int map_height = al_get_bitmap_height(gs->background);
    // if (Camera.y < 0)
    //     Camera.y = 0;
    // if (Camera.y > map_height - HEIGHT)
    //     Camera.y = map_height - HEIGHT;

    al_draw_bitmap(gs->background, -Camera.x, -Camera.y, 0);

    for (int i = 0; i < allEle.len; i++)
    {
        Elements* ele = allEle.arr[i];
        ele->Draw(ele);
    }
    SkillSystem_draw_menu(&gs->skill_sys);
}

// void game_scene_draw(Scene* self)
// {
//     al_clear_to_color(al_map_rgb(0, 0, 0));
//     GameScene *gs = ((GameScene *)(self->pDerivedObj));
//     al_draw_bitmap(gs->background, 0, 0, 0);
//     ElementVec allEle = _Get_all_elements(self);
//     for (int i = 0; i < allEle.len; i++)
//     {
//         Elements *ele = allEle.arr[i];
//         ele->Draw(ele);
//     }
// }

void game_scene_destroy(Scene* self)
{
    GameScene* Obj = ((GameScene*)(self->pDerivedObj));
    ALLEGRO_BITMAP* background = Obj->background;
    al_destroy_bitmap(background);
    ElementVec allEle = _Get_all_elements(self);
    for (int i = 0; i < allEle.len; i++)
    {
        Elements* ele = allEle.arr[i];
        ele->Destroy(ele);
    }
    free(Obj);
    free(self);
}
