#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include "character.h"
#include "projectile.h"
#include "weapon.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../algif5/algif.h"
#include "../scene/gamescene.h"
#include "../enemy/BasicEnemy.h"
#include <stdio.h>
#include <stdbool.h>

Elements *New_Character(int label)
{
    Character *pDerivedObj = (Character *)malloc(sizeof(Character));
    Elements *pObj = New_Elements(label);
    // load character images
    char state_string[7][10] = {"idle", "idled",
                                "up", "down", "left", "right", 
                                "die"};
    for (int i = 0; i < 7; i++)
    {
        char buffer[50];
        sprintf(buffer, "assets/image/chara_%s.gif", state_string[i]);
        pDerivedObj->gif_status[i] = algif_new_gif(buffer, -1);
        if (!pDerivedObj->gif_status[i]) {
            printf("Failed to load %s\n", buffer);
        }
    }
    // load effective sound
    ALLEGRO_SAMPLE *sample = al_load_sample("assets/sound/atk_sound.wav");
    pDerivedObj->atk_Sound = al_create_sample_instance(sample);
    al_set_sample_instance_playmode(pDerivedObj->atk_Sound, ALLEGRO_PLAYMODE_ONCE);
    al_attach_sample_instance_to_mixer(pDerivedObj->atk_Sound, al_get_default_mixer());

    // initial the geometric information of character
    pDerivedObj->hp = 300000;  // 初始血量
    pDerivedObj->width = pDerivedObj->gif_status[0]->width;
    pDerivedObj->height = pDerivedObj->gif_status[0]->height;
    pDerivedObj->x = 300;
    //pDerivedObj->y = HEIGHT - pDerivedObj->height - 60;
    pDerivedObj->y = HEIGHT/2;
    pDerivedObj->hitbox = New_Rectangle(
        pDerivedObj->x,
        pDerivedObj->y,
        pDerivedObj->x + pDerivedObj->width / 2,
        pDerivedObj->y + pDerivedObj->height / 2
    );
    pDerivedObj->dir = false; // true: face to right, false: face to left
    pDerivedObj->state = IDLE;
    pDerivedObj->last_move_dir = 0;
    pDerivedObj->new_proj = false;
    pObj->pDerivedObj = pDerivedObj;
    // setting derived object function
    pObj->Draw = Character_draw;
    pObj->Update = Character_update;
    pObj->Interact = Character_interact;
    pObj->Destroy = Character_destory;
    return pObj;
}

void Character_update(Elements *self) {
    Character *chara = ((Character *)(self->pDerivedObj));
    // 使用 last_move_dir 來記錄上一幀的上下移動
    // 0: 無, 1: 上, 2: 下
    // 優先判斷移動鍵
    if (key_state[ALLEGRO_KEY_W]) {
        chara->dir = false;
        _Character_update_position(self, 0, -5);
        chara->last_move_dir = 1; // 上
        chara->state = UP;
    }
    else if (key_state[ALLEGRO_KEY_A]) {
        chara->dir = false;
        _Character_update_position(self, -5, 0);
        chara->state = LEFT;
    }
    else if (key_state[ALLEGRO_KEY_S]) {
        chara->dir = false;
        _Character_update_position(self, 0, 5);
        chara->last_move_dir = 2; // 下
        chara->state = DOWN;
    }
    else if (key_state[ALLEGRO_KEY_D]) {
        chara->dir = false;
        _Character_update_position(self, 5, 0);
        chara->state = RIGHT;
    }
    else {
        // 根據上一個狀態決定 idle 顯示
        if (chara->last_move_dir == 1) {
            chara->state = IDLED; // 停止上移時顯示 idled
        } else if (chara->last_move_dir == 2) {
            chara->state = IDLE;  // 停止下移時顯示 idle
        } else {
            chara->state = IDLE;
        }
    }

    //printf("Character position: (%d, %d)\n", chara->x, chara->y);
    // idle動畫循環時自動攻擊
    if (chara->state == IDLE || chara->state == IDLED) {
        if ((chara->gif_status[IDLE] && chara->gif_status[IDLE]->display_index == 0 && chara->new_proj == false) ||
            (chara->gif_status[IDLED] && chara->gif_status[IDLED]->display_index == 0 && chara->new_proj == false)) {
            Character_fire_projectile(chara, chara->state); // <-- 呼叫 weapon.c 的函式
            chara->new_proj = true;
        }
        if ((chara->gif_status[IDLE] && chara->gif_status[IDLE]->done) ||
            (chara->gif_status[IDLED] && chara->gif_status[IDLED]->done)) {
            chara->new_proj = false;
        }
    }
/*
    // 動畫結束自動回到IDLE
    if (chara->gif_status[chara->state] && chara->gif_status[chara->state]->done) {
        chara->state = IDLE;
    }
*/
    if (chara->hp <= 0) {
        chara->state = DIE;
        if (chara->gif_status[DIE] && chara->gif_status[DIE]->done)
            self->dele = true;    
    }
}

void Character_draw(Elements *self)
{
    Character *chara = ((Character *)(self->pDerivedObj));
    ALLEGRO_BITMAP *frame = algif_get_bitmap(chara->gif_status[chara->state], al_get_time());
    int draw_x = chara->x + 20;
    int draw_y = chara->y - chara->height / 2 - 10;
    int weapon_x = chara->x + chara->width / 2 - 30;
    int weapon_y = chara->y - chara -> height / 3 + 10;

    if (chara->state == IDLE && chara->last_proj) {
        // idle 狀態：武器在角色後面
        Weapon_draw_rotated(weapon_x, weapon_y, chara->last_proj->angle);
        if (frame)
            al_draw_bitmap(frame, draw_x, draw_y, ((chara->dir) ? ALLEGRO_FLIP_HORIZONTAL : 0));
    } else if (chara->state == IDLED && chara->last_proj) {
        // idled 狀態：武器在角色前面
        if (frame)
            al_draw_bitmap(frame, draw_x, draw_y, ((chara->dir) ? ALLEGRO_FLIP_HORIZONTAL : 0));
        Weapon_draw_rotated(weapon_x, weapon_y, chara->last_proj->angle);
    } else {
        // 其他狀態只畫角色
        if (frame)
            al_draw_bitmap(frame, draw_x, draw_y, ((chara->dir) ? ALLEGRO_FLIP_HORIZONTAL : 0));
    }

    // idle動畫到特定幀時播放攻擊音效
    if ((chara->state == IDLE && chara->gif_status[IDLE] && chara->gif_status[IDLE]->display_index == 2 && chara->atk_Sound) ||
        (chara->state == IDLED && chara->gif_status[IDLED] && chara->gif_status[IDLED]->display_index == 2 && chara->atk_Sound))
    {
        al_play_sample_instance(chara->atk_Sound);
    }
}

void Character_destory(Elements *self)
{
    Character *Obj = ((Character *)(self->pDerivedObj));
    if (Obj->atk_Sound)
        al_destroy_sample_instance(Obj->atk_Sound);
    for (int i = 0; i < 7; i++)
        if (Obj->gif_status[i])
            algif_destroy_animation(Obj->gif_status[i]);
    if (Obj->hitbox)
        free(Obj->hitbox);
    free(Obj);
    free(self);
}

void _Character_update_position(Elements *self, int dx, int dy)
{
    Character* chara = ((Character*)(self->pDerivedObj));
    // GameScene* gs = ((GameScene*)(self->pDerivedObj));
    // int map_height = al_get_bitmap_height(gs->background);
    // chara->x += dx;
    // chara->y += dy;
    // Shape *hitbox = chara->hitbox;
    // if (hitbox) {
    //     hitbox->update_center_x(hitbox, dx);
    //     hitbox->update_center_y(hitbox, dy);
    // }
    int new_x = chara->x + dx;
    int new_y = chara->y + dy;

    // 限制角色活動範圍
    // 圖片高度為1024
    if (new_x < -16) new_x = -16;
    //if (new_x > WIDTH - chara->width) new_x = WIDTH - chara->width;
    if (new_x > 768) new_x = 768;
    if (new_y < 16) new_y = 16;
    if (new_y > HEIGHT) new_y = HEIGHT;
    //if (new_y > HEIGHT - chara->height) new_y = HEIGHT - chara->height;
    //if (new_y > HEIGHT+60 - chara->height) new_y = HEIGHT+60 - chara->height;
    //if (new_y > map_height - chara->height) new_y = map_height - chara->height;
    
    chara->x = new_x;
    chara->y = new_y;
}

void Character_interact(Elements *self) {
}