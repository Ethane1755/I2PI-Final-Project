#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include "character.h"
#include "projectile.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../algif5/algif.h"
#include "../scene/gamescene.h"
#include "../enemy/BasicEnemy.h"
#include <stdio.h>
#include <stdbool.h>

/*
   [Character function]
   合併 stop 和 atk 為 idle 狀態
   狀態只剩 idle, move
*/

Elements *New_Character(int label)
{
    Character *pDerivedObj = (Character *)malloc(sizeof(Character));
    Elements *pObj = New_Elements(label);
    // load character images
    char state_string[2][10] = {"idle", "move"};
    for (int i = 0; i < 2; i++)
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
    pDerivedObj->hp = 30;  // 初始血量
    pDerivedObj->width = pDerivedObj->gif_status[0]->width;
    pDerivedObj->height = pDerivedObj->gif_status[0]->height;
    pDerivedObj->x = 300;
    pDerivedObj->y = HEIGHT - pDerivedObj->height - 60;
    pDerivedObj->hitbox = New_Rectangle(
        pDerivedObj->x,
        pDerivedObj->y,
        pDerivedObj->x + pDerivedObj->width,
        pDerivedObj->y + pDerivedObj->height
    );
    pDerivedObj->dir = false; // true: face to right, false: face to left
    pDerivedObj->state = IDLE;
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
    // idle 狀態自動攻擊
    if (chara->state == IDLE) {
        // 有移動鍵就切換到 MOVE
        if (key_state[ALLEGRO_KEY_W]) {
            chara->dir = false;
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_A]) {
            chara->dir = false;
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_S]) {
            chara->dir = true;
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_D]) {
            chara->dir = true;
            chara->state = MOVE;
        }
        // idle動畫循環時自動攻擊
        if (chara->gif_status[IDLE] && chara->gif_status[IDLE]->display_index == 7 && chara->new_proj == false) {
            Elements *pro;
            if (chara->dir) {
                pro = New_Projectile(Projectile_L,
                                     chara->x - 50,
                                     chara->y + 10,
                                     -5);
            } else {
                pro = New_Projectile(Projectile_L,
                                     chara->x + chara->width - 100,
                                     chara->y + 10,
                                     5);
                
            }
            _Register_elements(scene, pro);
            chara->new_proj = true;
        }
        if (chara->gif_status[IDLE] && chara->gif_status[IDLE]->done) {
            chara->new_proj = false;
        }
    }
    else if (chara->state == MOVE) {
        if (key_state[ALLEGRO_KEY_W]) {
            chara->dir = false;
            _Character_update_position(self, 0, -5);
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_A]) {
            chara->dir = true;
            _Character_update_position(self, -5, 0);
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_S]) {
            chara->dir = false;
            _Character_update_position(self, 0, 5);
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_D]) {
            chara->dir = false;
            _Character_update_position(self, 5, 0);
            chara->state = MOVE;
        }
        // 沒有移動鍵就回到 IDLE
        else {
            chara->state = IDLE;
        }
        if (chara->gif_status[MOVE] && chara->gif_status[MOVE]->done)
            chara->state = IDLE;
    }
    if (chara->hp <= 0) {
        self->dele = true; 
    }
}

void Character_draw(Elements *self)
{
    Character *chara = ((Character *)(self->pDerivedObj));
    ALLEGRO_BITMAP *frame = algif_get_bitmap(chara->gif_status[chara->state], al_get_time());
    if (frame)
    {
        al_draw_bitmap(frame, chara->x, chara->y - chara -> height / 2 + 23, ((chara->dir) ? ALLEGRO_FLIP_HORIZONTAL : 0));
    }
    // idle動畫到特定幀時播放攻擊音效
    if (chara->state == IDLE && chara->gif_status[IDLE] && chara->gif_status[IDLE]->display_index == 2 && chara->atk_Sound)
    {
        al_play_sample_instance(chara->atk_Sound);
    }
}

void Character_destory(Elements *self)
{
    Character *Obj = ((Character *)(self->pDerivedObj));
    if (Obj->atk_Sound)
        al_destroy_sample_instance(Obj->atk_Sound);
    for (int i = 0; i < 2; i++)
        if (Obj->gif_status[i])
            algif_destroy_animation(Obj->gif_status[i]);
    if (Obj->hitbox)
        free(Obj->hitbox);
    free(Obj);
    free(self);
}

void _Character_update_position(Elements *self, int dx, int dy)
{
    Character *chara = ((Character *)(self->pDerivedObj));
    chara->x += dx;
    chara->y += dy;
    Shape *hitbox = chara->hitbox;
    if (hitbox) {
        hitbox->update_center_x(hitbox, dx);
        hitbox->update_center_y(hitbox, dy);
    }
}

void Character_interact(Elements *self) {
}
