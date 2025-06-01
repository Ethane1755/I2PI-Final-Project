#include <allegro5/allegro_audio.h>
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
/*
   [GameScene function]
*/
// Character character; //character

Scene* New_GameScene(int label)
{
    GameScene* pDerivedObj = (GameScene*)malloc(sizeof(GameScene));
    Scene* pObj = New_Scene(label);
    // setting derived object member
    pDerivedObj->background = al_load_bitmap("assets/image/map2.png");
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
        // update every element
    ElementVec allEle = _Get_all_elements(self);
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

}

void game_scene_draw(Scene* self)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));
    GameScene* gs = ((GameScene*)(self->pDerivedObj));

    // 取得角色座標
    ElementVec allEle = _Get_all_elements(self);
    //int chara_x = 0;
    int chara_y = 0;
    for (int i = 0; i < allEle.len; i++) {
        Elements* ele = allEle.arr[i];
        if (ele->label == Character_L) {
            Character* chara = (Character*)(ele->pDerivedObj);
            //chara_x = chara->x;
            chara_y = chara->y;
            break;
        }
    }

    //printf("Character position: (%d, %d)\n", chara_x, chara_y);

    // 計算 Camera
    Point Camera;
    Camera.x = 0; // x 固定不動
    Camera.y = chara_y;
    // if(chara_y<600)
    //     Camera.y = chara_y;
    // else
    //     Camera.y = 1024;

    // 限制 Camera.y 不超過地圖上下邊界
    if (Camera.y < 0)
        Camera.y = 0;
    if (Camera.y > HEIGHT)
        Camera.y = HEIGHT;
    //printf("Camera.y=%lf\n", Camera.y);
    // 背景根據 Camera 偏移
    al_draw_bitmap(gs->background, -Camera.x, -Camera.y, 0);

    for (int i = 0; i < allEle.len; i++)
    {
        Elements* ele = allEle.arr[i];
        ele->Draw(ele);            
    }
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
