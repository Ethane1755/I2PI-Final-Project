#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include "character.h"
#include "projectile.h"
#include "weapon.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../algif5/algif.h"
#include "../scene/gamescene.h"
#include "../enemy/BasicEnemy.h"
#include "../enemy/BossEnemy.h"
#include "../enemy/BulletEnemy.h"
#include "../enemy/TraceEnemy.h"
#include <stdio.h>
#include <stdbool.h>

#include "elementLabel.h"

#include <math.h> // for ALLEGRO_PI


ElementVec _Get_all_the_enemies(Scene* scene) {
    ElementVec allEnemies;
    allEnemies.len = 0;

    EleType enemyTypes[] = { BasicEnemy_L, BulletEnemy_L, TraceEnemy_L, BossEnemy_L };
    int numEnemyTypes = sizeof(enemyTypes) / sizeof(EleType);

    for (int t = 0; t < numEnemyTypes; t++) {
        ElementVec enemies = _Get_label_elements(scene, enemyTypes[t]);
        for (int i = 0; i < enemies.len; i++) {
            if (allEnemies.len < MAX_ELEMENT) {
                allEnemies.arr[allEnemies.len++] = enemies.arr[i];
            }
            else {
                break;
            }
        }
        if (allEnemies.len >= MAX_ELEMENT) {
            break;
        }
    }

    return allEnemies;
}

Elements* New_Character(int label)
{
    Character* pDerivedObj = (Character*)malloc(sizeof(Character));
    Elements* pObj = New_Elements(label);
    // load character images
    char state_string[7][10] = { "idle", "idled",
                                "up", "down", "left", "right",
                                "die" };
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
    ALLEGRO_SAMPLE* sample = al_load_sample("assets/sound/atk_sound.wav");
    pDerivedObj->atk_Sound = al_create_sample_instance(sample);
    al_set_sample_instance_playmode(pDerivedObj->atk_Sound, ALLEGRO_PLAYMODE_ONCE);
    al_attach_sample_instance_to_mixer(pDerivedObj->atk_Sound, al_get_default_mixer());

    // initial the geometric information of character
    pDerivedObj->width = pDerivedObj->gif_status[0]->width;
    pDerivedObj->height = pDerivedObj->gif_status[0]->height;
    pDerivedObj->x = 300;
    pDerivedObj->y = HEIGHT / 2;
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
    
    // Initialize health and skills
    pDerivedObj->max_hp = 100;  // Set a reasonable max HP
    pDerivedObj->hp = pDerivedObj->max_hp;  // Start with full health
    pDerivedObj->shield_active = false;
    pDerivedObj->speed = 1.0f;
    pDerivedObj->multi_shot = false;
    pDerivedObj->multishot_level = 0;
    pDerivedObj->pending_multishot = false;
    pDerivedObj->last_multishot_time = 0;
    pDerivedObj->multishot_shots_left = 0;
    pDerivedObj->last_proj = NULL;
    pObj->pDerivedObj = pDerivedObj;
    // setting derived object function
    pObj->Draw = Character_draw;
    pObj->Update = Character_update;
    pObj->Interact = Character_interact;
    pObj->Destroy = Character_destory;
    return pObj;
}

void Character_update(Elements* self) {
    Character* chara = ((Character*)(self->pDerivedObj));
    // 使用 last_move_dir 來記錄上一幀的上下移動
    // 0: 無, 1: 上, 2: 下
    // 優先判斷移動鍵
    int move_speed = (int)(5 * chara->speed);
    if (key_state[ALLEGRO_KEY_W]) {
        chara->dir = false;
        _Character_update_position(self, 0, -move_speed);
        chara->last_move_dir = 1; // 上
        chara->state = UP;
    }
    else if (key_state[ALLEGRO_KEY_A]) {
        chara->dir = false;
        _Character_update_position(self, -move_speed, 0);
        chara->state = LEFT;
    }
    else if (key_state[ALLEGRO_KEY_S]) {
        chara->dir = false;
        _Character_update_position(self, 0, move_speed);
        chara->last_move_dir = 2; // 下
        chara->state = DOWN;
    }
    else if (key_state[ALLEGRO_KEY_D]) {
        chara->dir = false;
        _Character_update_position(self, move_speed, 0);
        chara->state = RIGHT;
    }
    else {
        // 根據上一個狀態決定 idle 顯示
        if (chara->last_move_dir == 1) {
            chara->state = IDLED; // 停止上移時顯示 idled
        }
        else if (chara->last_move_dir == 2) {
            chara->state = IDLE;  // 停止下移時顯示 idle
        }
        else {
            chara->state = IDLE;
        }
    }

    //printf("Character position: (%d, %d)\n", chara->x, chara->y);
    // idle動畫循環時自動攻擊
    ElementVec enemies = _Get_all_the_enemies(scene);
    for (int i = 0; i < enemies.len; i++) {
        Elements* e = enemies.arr[i];
        if (enemies.arr[i]->label == BossEnemy_L) {
            BossEnemy* enemyObj = (BossEnemy*)(e->pDerivedObj);
            if (enemyObj->is_invisible) return;
            if (chara->state == IDLE || chara->state == IDLED) {
                if ((chara->gif_status[IDLE] && chara->gif_status[IDLE]->display_index == 0 && chara->new_proj == false) ||
                    (chara->gif_status[IDLED] && chara->gif_status[IDLED]->display_index == 0 && chara->new_proj == false)) {
                    Character_fire_projectile(chara, chara->state); // <-- 呼叫 weapon.c 的函式
                    chara->new_proj = true;
                    if (chara->multi_shot && chara->multishot_level > 0) {
                        chara->pending_multishot = true;
                        chara->last_multishot_time = al_get_time();
                        chara->multishot_shots_left = chara->multishot_level;
                    }
                }
                if ((chara->gif_status[IDLE] && chara->gif_status[IDLE]->done) ||
                    (chara->gif_status[IDLED] && chara->gif_status[IDLED]->done)) {
                    chara->new_proj = false;
                }
            }
        }
        else {
            if (chara->state == IDLE || chara->state == IDLED) {
                if ((chara->gif_status[IDLE] && chara->gif_status[IDLE]->display_index == 0 && chara->new_proj == false) ||
                    (chara->gif_status[IDLED] && chara->gif_status[IDLED]->display_index == 0 && chara->new_proj == false)) {
                    Character_fire_projectile(chara, chara->state); // <-- 呼叫 weapon.c 的函式
                    chara->new_proj = true;
                    if (chara->multi_shot && chara->multishot_level > 0) {
                        chara->pending_multishot = true;
                        chara->last_multishot_time = al_get_time();
                        chara->multishot_shots_left = chara->multishot_level;
                    }
                }
                if ((chara->gif_status[IDLE] && chara->gif_status[IDLE]->done) ||
                    (chara->gif_status[IDLED] && chara->gif_status[IDLED]->done)) {
                    chara->new_proj = false;
                }
            }
        }
    }

    // Multi Shot: fire another projectile after 0.1s
    if (chara->multi_shot && chara->pending_multishot && chara->multishot_shots_left > 0) {
        if (al_get_time() - chara->last_multishot_time >= 0.1) {
            Character_fire_projectile(chara, chara->state);
            chara->last_multishot_time = al_get_time();
            chara->multishot_shots_left--;
            if (chara->multishot_shots_left == 0) {
                chara->pending_multishot = false;
            }
        }
    }

    if (chara->hp <= 0) {
        chara->state = DIE;
        if (chara->gif_status[DIE] && chara->gif_status[DIE]->done)
            self->dele = true;
    }
}

void Character_draw(Elements* self)
{
    Character* chara = ((Character*)(self->pDerivedObj));
    ALLEGRO_BITMAP* frame = algif_get_bitmap(chara->gif_status[chara->state], al_get_time());
    int draw_x = chara->x+20 ;
    int draw_y = chara->y - chara->height / 2 - 10;
    int weapon_x = chara->x + chara->width / 2 - 35;
    int weapon_y = chara->y - chara->height / 3 + 10;

    // 判斷有沒有 last_proj
    float weapon_angle = 0;
    bool has_proj = (chara->last_proj != NULL);

    if (chara->state == IDLE) {
        if (has_proj)
            weapon_angle = chara->last_proj->angle; // 跟 bullet 方向
        else
            weapon_angle = -ALLEGRO_PI / 2; // 預設朝上
        Weapon_draw_rotated(weapon_x, weapon_y, weapon_angle);
        if (frame)
            al_draw_bitmap(frame, draw_x, draw_y, ((chara->dir) ? ALLEGRO_FLIP_HORIZONTAL : 0));
    }
    else if (chara->state == IDLED) {
        if (frame)
            al_draw_bitmap(frame, draw_x, draw_y, ((chara->dir) ? ALLEGRO_FLIP_HORIZONTAL : 0));
        if (has_proj)
            weapon_angle = chara->last_proj->angle; // 跟 bullet 方向
        else
            weapon_angle = ALLEGRO_PI / 2; // 預設朝下
        Weapon_draw_rotated(weapon_x, weapon_y, weapon_angle);
    }
    else {
        if (frame)
            al_draw_bitmap(frame, draw_x, draw_y, ((chara->dir) ? ALLEGRO_FLIP_HORIZONTAL : 0));
    }

    if (chara->shield_active) {
        static ALLEGRO_BITMAP* shield_img = NULL;
        if (!shield_img)
            shield_img = al_load_bitmap("assets/image/shield.png");
        if (shield_img)
            al_draw_tinted_bitmap(shield_img, al_map_rgba_f(1,1,1,0.5), chara->x, chara->y - chara->height / 2, 0);
    }

    // idle動畫到特定幀時播放攻擊音效
    if ((chara->state == IDLE && chara->gif_status[IDLE] && chara->gif_status[IDLE]->display_index == 2 && chara->atk_Sound) ||
        (chara->state == IDLED && chara->gif_status[IDLED] && chara->gif_status[IDLED]->display_index == 2 && chara->atk_Sound))
    {
        al_play_sample_instance(chara->atk_Sound);
    }
}

void Character_destory(Elements* self)
{
    Character* Obj = ((Character*)(self->pDerivedObj));
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

void _Character_update_position(Elements* self, int dx, int dy)
{
    Character* chara = ((Character*)(self->pDerivedObj));
    int new_x = chara->x + dx;
    int new_y = chara->y + dy;

    // 限制角色活動範圍
    if (new_x < -16) new_x = -16;
    if (new_x > 768) new_x = 768;
    if (new_y < 16) new_y = 16;
    if (new_y > HEIGHT-10) new_y = HEIGHT-10;

    chara->x = new_x;
    chara->y = new_y;
}

void Character_take_damage(Elements* self, int damage) {
    Character* chara = (Character*)(self->pDerivedObj);
    if (chara->shield_active) {
        chara->shield_active = false;
        printf("Shield blocked the attack!\n");
    } else {
        chara->hp -= damage;
        printf("Player hit! HP: %d\n", chara->hp);
    }
}

void Character_interact(Elements* self) {
    // 角色互動目前無需實作
}