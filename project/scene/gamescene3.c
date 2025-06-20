#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_primitives.h> //
#include <allegro5/allegro_acodec.h> //
#include <allegro5/allegro_font.h> //
#include <allegro5/allegro_ttf.h> //
#include "gamescene3.h"
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

Scene* New_GameScene3(int label)
{
    printf("New_GameScene3 called, label=%d\n", label);
    GameScene3* pDerivedObj = (GameScene3*)malloc(sizeof(GameScene3));
    Scene* pObj = New_Scene(label);
    //printf("New GameScene2 has created.\n");
    // setting derived object member
    pDerivedObj->background = al_load_bitmap("assets/image/second_map.png");
    SkillSystem_init(&pDerivedObj->skill_sys);
    pObj->pDerivedObj = pDerivedObj;
    // register element
    //_Register_elements(pObj, New_Floor(Floor_L));
    _Register_elements(pObj, New_Teleport(Teleport_L));
    //_Register_elements(pObj, New_Tree(Tree_L));
    _Register_elements(pObj, New_Character(Character_L));
    _Register_elements(pObj, New_Ball(Ball_L));
    //_Register_elements(pObj, New_BasicEnemy(BasicEnemy_L, 100, 100));
    //_Register_elements(pObj, New_BulletEnemy(BulletEnemy_L, 200, 200));
    //_Register_elements(pObj, New_TraceEnemy(TraceEnemy_L, 300, 300));
    _Register_elements(pObj, New_BossEnemy(BossEnemy_L, 200, 200));
    // setting derived object function
    pObj->Update = game_scene3_update;
    pObj->Draw = game_scene3_draw;
    pObj->Destroy = game_scene3_destroy;
    return pObj;
}
void game_scene3_update(Scene* self)
{
    GameScene3* gs = (GameScene3*)self->pDerivedObj;

    // update every element
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

    ElementVec allEnemies = _Get_all_enemies(self);


    //gs->timer = 0;  
    if (allEnemies.len == 0) {
        //gs->timer = 0;
        self->scene_end = true;
        window = 4;
        return;
    }

    // ElementVec allEle = _Get_all_elements(self);
    // for (int i = 0; i < allEle.len; i++) {
    //     Elements* ele = allEle.arr[i];
    //     ele->Destroy(ele);
    // }
    // if (gs->background) {
    //     al_destroy_bitmap(gs->background);
    //     gs->background = NULL;
    // }
    // if (!gs->win_img)
    //     gs->win_img = al_load_bitmap("assets/image/win.png");


    // //gs->timer += 1.0 / 60.0; // 假設你每幀呼叫一次，且 FPS=60
    // if (key_state[ALLEGRO_KEY_ENTER]) {
    //     //gs->timer = 0;
    //     self->scene_end = true;
    //     window = 3;
    //     return;
    // }


}

// Helper function to draw health bars
void draw_health_bar3(int x, int y, int width, int height, float current_hp, float max_hp, 
                    ALLEGRO_COLOR border_color, ALLEGRO_COLOR fill_color, bool show_text) {
    // 確保血量不會小於0
    if (current_hp < 0) current_hp = 0;
    if (max_hp <= 0) max_hp = 1; // 避免除以零
    
    // 繪製陰影效果
    al_draw_filled_rectangle(x + 2, y + 2, x + width + 2, y + height + 2, 
                            al_map_rgba(0, 0, 0, 80));
    
    // 繪製背景（空血條）
    al_draw_filled_rectangle(x, y, x + width, y + height, al_map_rgb(60, 60, 60));
    
    // 計算填充寬度
    float health_ratio = current_hp / max_hp;
    float fill_width = health_ratio * width;
    
    // 根據血量百分比改變顏色
    ALLEGRO_COLOR health_color;
    if (health_ratio <= 0.2f) {
        health_color = al_map_rgb(255, 50, 50);     // 紅色 (危險)
    } else if (health_ratio <= 0.5f) {
        health_color = al_map_rgb(255, 150, 50);    // 橙色 (警告)
    } else if (health_ratio <= 0.8f) {
        health_color = al_map_rgb(255, 255, 50);    // 黃色 (注意)
    } else {
        health_color = al_map_rgb(50, 255, 50);     // 綠色 (健康)
    }
    
    // 繪製血條填充部分
    if (fill_width > 0) {
        al_draw_filled_rectangle(x, y, x + fill_width, y + height, health_color);
        
        // 添加光澤效果
        al_draw_filled_rectangle(x, y, x + fill_width, y + height/3, 
                                al_map_rgba(255, 255, 255, 30));
    }
    
    // 繪製邊框
    al_draw_rectangle(x - 1, y - 1, x + width + 1, y + height + 1, 
                     al_map_rgb(255, 255, 255), 1);
    al_draw_rectangle(x, y, x + width, y + height, border_color, 1);
    
    // 繪製血量文字
    if (show_text) {
        static ALLEGRO_FONT* font = NULL;
        if (!font) {
            font = al_load_font("assets/font/pirulen.ttf", 14, 0);
            if (!font) {
                font = al_create_builtin_font();
            }
        }
        if (font) {
            char hp_text[32];
            sprintf(hp_text, "%d/%d", (int)current_hp, (int)max_hp);
            
            // 文字陰影
            al_draw_text(font, al_map_rgb(0, 0, 0), x + width/2 + 1, y + height/2 - 6 + 1,
                        ALLEGRO_ALIGN_CENTER, hp_text);
            // 文字本體
            al_draw_text(font, al_map_rgb(255, 255, 255), x + width/2, y + height/2 - 6,
                        ALLEGRO_ALIGN_CENTER, hp_text);
        }
    }
}

void game_scene3_draw(Scene* self)
{
    //printf("Drawing GameScene2\n");
    al_clear_to_color(al_map_rgb(0, 0, 0));
    GameScene3* gs = ((GameScene3*)(self->pDerivedObj));

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

    if (Camera.y < 0)
        Camera.y = 0;
    if (Camera.y > HEIGHT)
        Camera.y = HEIGHT;

    al_draw_bitmap(gs->background, -Camera.x, -Camera.y, 0);

    for (int i = 0; i < allEle.len; i++)
    {
        Elements* ele = allEle.arr[i];
        ele->Draw(ele);
        
        // 绘制玩家血条（固定在屏幕左上角）
        if (ele->label == Character_L) {
            Character* character = (Character*)(ele->pDerivedObj);
            draw_health_bar3(15, 15, 250, 25, character->hp, character->max_hp, 
                          al_map_rgb(200, 200, 200), al_map_rgb(0, 255, 0), true);
        }
        // 绘制敌人血条
        else if (ele->label == BasicEnemy_L || ele->label == BulletEnemy_L || 
                 ele->label == TraceEnemy_L || ele->label == BossEnemy_L) {
            
            // 根据敌人类型设定血条参数
            int bar_width, bar_height;
            float max_hp;
            ALLEGRO_COLOR enemy_color;
            
            if (ele->label == BossEnemy_L) {
                // 对于Boss敌人，使用BossEnemy结构
                BossEnemy* boss = (BossEnemy*)(ele->pDerivedObj);
                
                // Don't show health bar if boss is dead/dying, has no health, or in stealth
                if (boss->state == BOSS_BE_STATE_DIE || 
                    boss->state == BOSS_BE_STATE_STEALTH_IN ||
                    boss->state == BOSS_BE_STATE_STEALTH_OUT ||
                    boss->hp <= 0 ||
                    boss->is_invisible) {
                    continue;
                }
                
                bar_width = 120;
                bar_height = 8;
                max_hp = 25.0f;
                enemy_color = al_map_rgb(255, 0, 255);  // 紫色 (Boss)
                
                // 计算血条位置（Boss正上方）
                int bar_x = boss->x + (boss->width / 2) - (bar_width / 2);
                int bar_y = boss->y - bar_height - 8;
                
                // 确保血条不会超出屏幕边界
                if (bar_x < 5) bar_x = 5;
                if (bar_x + bar_width > WIDTH - 5) bar_x = WIDTH - bar_width - 5;
                if (bar_y < 5) bar_y = boss->y + boss->height + 3;
                
                // 只有当Boss血量小于最大值时才显示血条
                if (boss->hp < max_hp) {
                    draw_health_bar3(bar_x, bar_y, bar_width, bar_height, 
                                  boss->hp, max_hp, enemy_color, enemy_color, false);
                }
            }
            else {
                // 对于其他敌人类型，使用BasicEnemy结构
                BasicEnemy* enemy = (BasicEnemy*)(ele->pDerivedObj);
                
                switch (ele->label) {
                    case TraceEnemy_L:
                        bar_width = 70;
                        bar_height = 6;
                        max_hp = 10.0f;
                        enemy_color = al_map_rgb(255, 100, 0);  // 橙色 (追踪敌人)
                        break;
                    case BulletEnemy_L:
                        bar_width = 60;
                        bar_height = 5;
                        max_hp = 8.0f;
                        enemy_color = al_map_rgb(255, 255, 0);  // 黄色 (射击敌人)
                        break;
                    default: // BasicEnemy_L
                        bar_width = 50;
                        bar_height = 4;
                        max_hp = 5.0f;
                        enemy_color = al_map_rgb(255, 0, 0);    // 红色 (基本敌人)
                        break;
                }
                
                // 计算血条位置（敌人正上方）
                int bar_x = enemy->x + (enemy->width / 2) - (bar_width / 2);
                int bar_y = enemy->y - bar_height - 8;
                
                // 确保血条不会超出屏幕边界
                if (bar_x < 5) bar_x = 5;
                if (bar_x + bar_width > WIDTH - 5) bar_x = WIDTH - bar_width - 5;
                if (bar_y < 5) bar_y = enemy->y + enemy->height + 3;
                
                // 只有当敌人血量小于最大值时才显示血条
                if (enemy->hp < max_hp) {
                    draw_health_bar3(bar_x, bar_y, bar_width, bar_height, 
                                  enemy->hp, max_hp, enemy_color, enemy_color, false);
                }
            }
        }
    }
    SkillSystem_draw_menu(&gs->skill_sys);
}

void game_scene3_destroy(Scene* self)
{
    GameScene3* Obj = ((GameScene3*)(self->pDerivedObj));
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
