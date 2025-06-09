/* BossEnemy.c - 修正版本 */
#include "BossEnemy.h"
#include "../element/character.h"
#include "../element/projectile.h"
#include "../shapes/Rectangle.h"
#include "../scene/sceneManager.h"
#include "../scene/gamescene.h"
#include <allegro5/allegro_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../shapes/Point.h"
#include "../element/elementLabel.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//extern Point Camera;

bool isPositionValid(float x, float y, int width, int height) {
    // if (x < 0 || y < 0 || x + width > 1440 || y + height > 900) {
    //     return false;
    // }

    //since HEIGHT=672 and WIDTH=900, reference in global.c
    if (x < 0 || y < 0 || x + width > 900 || y + height > 672) {
        return false;
    }

    return true;
}

void findValidTeleportPosition(float px, float py, int width, int height, float *new_x, float *new_y, bool *new_dir) {
    float offset = 150;
    float positions[][3] = {
        {px + offset, py - height/2, 1},     // 右側，面向左
        {px - offset - width, py - height/2, 0}, // 左側，面向右
        {px - width/2, py + offset, 0},      // 下方，面向右
        {px - width/2, py - offset - height, 0}, // 上方，面向右
    };

    for (int i = 0; i < 4; i++) {
        if (isPositionValid(positions[i][0], positions[i][1], width, height)) {
            *new_x = positions[i][0];
            *new_y = positions[i][1];
            *new_dir = (bool)positions[i][2];
            return;
        }
    }

    for (int radius = 100; radius <= 300; radius += 50) {
        for (int angle = 0; angle < 360; angle += 45) {
            float rad = angle * M_PI / 180.0;
            float test_x = px + cos(rad) * radius - width/2;
            float test_y = py + sin(rad) * radius - height/2;
            
            if (isPositionValid(test_x, test_y, width, height)) {
                *new_x = test_x;
                *new_y = test_y;
                *new_dir = (test_x > px); // 如果在玩家右側，面向左
                return;
            }
        }
    }

    *new_x = px - width/2;
    *new_y = py - height/2;
    *new_dir = false;
}

Elements *New_BossEnemy(int label, int x, int y) {
    Elements *obj = New_Elements(label);
    BossEnemy *e = calloc(1, sizeof(BossEnemy));
    obj->pDerivedObj = e;

    char path[100];
    for (int i = 0; i < 8; i++) {
        sprintf(path, "assets/image/Bringer-of-Death_Walk_%d.png", i+1);
        e->walk_frames[i] = al_load_bitmap(path);
    }
    for (int i = 0; i < 10; i++) {
        sprintf(path, "assets/image/Bringer-of-Death_Attack_%d.png", i+1);
        e->attack_frames[i] = al_load_bitmap(path);
    }
    for (int i = 0; i < 10; i++) {
        sprintf(path, "assets/image/Bringer-of-Death_Death_%d.png", i+1);
        e->death_frames[i] = al_load_bitmap(path);
    }
    for (int i = 0; i < 9; i++) {
        sprintf(path, "assets/image/Bringer-of-Death_Cast_%d.png", i+1);
        e->cast_frames[i] = al_load_bitmap(path);
    }
    for (int i = 0; i < 16; i++) {
        sprintf(path, "assets/image/Bringer-of-Death_Spell_%d.png", i+1);
        e->spell_frames[i] = al_load_bitmap(path);
    }
    for (int i = 0; i < 3; i++) {
        sprintf(path, "assets/image/Bringer-of-Death_Hurt_%d.png", i+1);
        e->hurt_frames[i] = al_load_bitmap(path);
    }
    e->die_bitmap = al_load_bitmap("assets/image/Boss-Die.png");

    e->x = x;
    e->y = y;
    e->width = al_get_bitmap_width(e->walk_frames[0]) * 2;
    e->height = al_get_bitmap_height(e->walk_frames[0]) * 2;
    e->hitbox = New_Rectangle(x, y, x + e->width, y + e->height);
    e->hp = 30;
    e->damage = 5;
    e->dir = false;
    e->frame = 0;
    e->frame_duration = 0.1;
    e->last_frame_time = al_get_time();
    e->state = BOSS_BE_STATE_WALK;
    
    e->last_stealth_time = al_get_time();
    e->stealth_interval = 12.0; // 12秒間隔
    e->stealth_duration = 4.0;  // 潛行持續4秒
    e->in_stealth = false;
    e->is_invisible = false;
    e->stealth_start_time = 0; 
    
    // 修改：改為累計攻擊次數
    e->total_attack_count = 0;      // 總攻擊次數
    e->skill_delay = 2.0;           // 技能延遲時間
    e->pending_skill = false;       // 是否有待釋放的技能
    e->skill_trigger_time = 0;      // 技能觸發時間
    
    e->cast_completed = false;
    e->last_damage_time = 0;
    e->invincible_time = 0.5;       // 受傷後0.5秒無敵

    obj->inter_obj[0] = Character_L;
    obj->inter_obj[1] = Projectile_L;
    obj->inter_len = 2;

    obj->Update = BossEnemy_update;
    obj->Draw = BossEnemy_draw;
    obj->Interact = BossEnemy_interact;
    obj->Destroy = BossEnemy_destroy;
    return obj;
}

void BossEnemy_update(Elements *self) {
    BossEnemy *e = self->pDerivedObj;
    double now = al_get_time();

    if (e->state == BOSS_BE_STATE_DIE) {
        if (now - e->last_frame_time > e->frame_duration) {
            e->frame++;
            if (e->frame >= 14) { 
                self->dele = true;
                return;
            }
            e->last_frame_time = now;
        }
        return;
    }

    if (e->state == BOSS_BE_STATE_HURT) {
        if (now - e->last_frame_time > e->frame_duration) {
            e->frame++;
            if (e->frame >= 3) {
                e->state = BOSS_BE_STATE_WALK;
                e->frame = 0;
            }
            e->last_frame_time = now;
        }
        return;
    }

    ElementVec all = _Get_all_elements(scene);
    Elements *player = NULL;
    for (int i = 0; i < all.len; i++) {
        if (all.arr[i]->label == Character_L) {
            player = all.arr[i];
            break;
        }
    }
    if (!player) return;

    Character *c = (Character *)(player->pDerivedObj);
    float px = c->x + c->width/2;
    float py = c->y + c->height/2;
    float bx = e->x + e->width/2;
    float by = e->y + e->height/2;
    float dx = px - bx;
    float dy = py - by;
    float dist = sqrt(dx*dx + dy*dy);

    // 檢查是否需要觸發技能（第3次攻擊後延遲2秒）
    if (e->pending_skill && now - e->skill_trigger_time >= e->skill_delay) {
        e->state = BOSS_BE_STATE_CAST;
        e->frame = 0;
        e->last_frame_time = now;
        e->cast_completed = false;
        e->pending_skill = false;
        printf("Boss casts skill after delay! Total attacks: %d\n", e->total_attack_count);
        return;
    }

    // 潛行邏輯保持不變
    if (!e->in_stealth && now - e->last_stealth_time > e->stealth_interval && 
        e->state == BOSS_BE_STATE_WALK && !e->pending_skill) {
        e->state = BOSS_BE_STATE_STEALTH_IN;
        e->in_stealth = true;
        e->is_invisible = true; 
        e->stealth_start_time = now; 
        e->frame = 0;
        e->last_frame_time = now;
        printf("Boss enters stealth mode!\n");
        return;
    }

    if (e->state == BOSS_BE_STATE_STEALTH_IN) {
        if (now - e->last_frame_time > e->frame_duration) {
            e->frame++;
            if (e->frame >= 10) {
                e->frame = 10; 
            }
            e->last_frame_time = now;
        }

        if (now - e->stealth_start_time >= e->stealth_duration) {
            float new_x, new_y;
            bool new_dir;
            findValidTeleportPosition(px, py, e->width, e->height, &new_x, &new_y, &new_dir);
            
            e->x = new_x;
            e->y = new_y;
            e->dir = new_dir;
            
            e->state = BOSS_BE_STATE_STEALTH_OUT;
            e->frame = 9; 
            printf("Boss teleports to safe position near player!\n");
        }
        return;
    }

    if (e->state == BOSS_BE_STATE_STEALTH_OUT) {
        if (now - e->last_frame_time > e->frame_duration) {
            e->frame--;
            if (e->frame < 0) {
                e->state = BOSS_BE_STATE_WALK;
                e->in_stealth = false;
                e->is_invisible = false;  
                e->last_stealth_time = now;
                e->frame = 0;
                printf("Boss exits stealth mode!\n");
            }
            e->last_frame_time = now;
        }
        return;
    }

    if (e->state == BOSS_BE_STATE_CAST) {
        if (now - e->last_frame_time > e->frame_duration) {
            e->frame++;
            if (e->frame >= 9) {
                if (!e->cast_completed) {
                    e->target_x = px - e->width/2;
                    e->target_y = py - e->height/2;
                    e->cast_completed = true;
                    printf("Boss targets spell at player position!\n");
                }
                e->state = BOSS_BE_STATE_SPELL;
                e->frame = 0;
            }
            e->last_frame_time = now;
        }
        return;
    }

    if (e->state == BOSS_BE_STATE_SPELL) {
        if (now - e->last_frame_time > e->frame_duration) {
            e->frame++;
            if (e->frame == 8 && e->cast_completed) {
                float spell_range = 150; // 法術傷害範圍
                float current_dx = px - (e->target_x + e->width/2);
                float current_dy = py - (e->target_y + e->height/2);
                float current_dist = sqrt(current_dx*current_dx + current_dy*current_dy);
                if (current_dist < spell_range) {
                    Character_take_damage(player, e->damage * 2);  // 技能傷害是普攻的2倍
                    printf("Player hit by boss spell! HP: %d\n", c->hp);
                }
            }
            if (e->frame >= 16) {
                e->state = BOSS_BE_STATE_WALK;
                e->frame = 0;
                e->cast_completed = false;
                // 技能釋放後重置攻擊計數
                e->total_attack_count = 0;
                printf("Boss skill completed, attack count reset\n");
            }
            e->last_frame_time = now;
        }
        return;
    }

    if (e->state == BOSS_BE_STATE_ATTACK) {
        if (now - e->last_frame_time > e->frame_duration) {
            e->frame++;
            if (e->frame == 5) {
                float attack_range = 120;
                if (dist < attack_range) {
                    Character_take_damage(player, e->damage * 2);
                    printf("Player hit by boss attack! HP: %d\n", c->hp);
                }
            }
            if (e->frame >= 10) {
                e->state = BOSS_BE_STATE_WALK;
                e->frame = 0;
                e->total_attack_count++;
                printf("Boss total attack count: %d\n", e->total_attack_count);
                
                // 檢查是否達到第3次攻擊，設置延遲技能觸發
                if (e->total_attack_count % 3 == 0) {
                    e->pending_skill = true;
                    e->skill_trigger_time = now;
                    printf("Boss will cast skill in %.1f seconds!\n", e->skill_delay);
                }
            }
            e->last_frame_time = now;
        }
        return;
    }

    if (e->state == BOSS_BE_STATE_WALK) {
        // 修改：只要距離夠近就可以攻擊，不再需要檢查combo
        if (dist < 120) {
            e->state = BOSS_BE_STATE_ATTACK;
            e->frame = 0;
            e->last_frame_time = now;
            printf("Boss performs attack (total: %d)\n", e->total_attack_count + 1);
        } else if (dist < 400) {
            float move_speed = 1.0;
            e->x += (dx / dist) * move_speed;
            e->y += (dy / dist) * move_speed;
            
            e->dir = (dx > 0); 
        }

        if (now - e->last_frame_time > e->frame_duration) {
            e->frame = (e->frame + 1) % 8;
            e->last_frame_time = now;
        }
    }

    if (e->hitbox) {
        e->hitbox->update(e->hitbox, e->x, e->y, e->x + e->width, e->y + e->height);
    }

    //printf("Boss draw at: (%.1f, %.1f)\n", e->x, e->y);
}

void BossEnemy_draw(Elements *self) {
    BossEnemy *e = self->pDerivedObj;
    ALLEGRO_BITMAP *bmp = NULL;
    float draw_x = e->x;  
    
    if (e->is_invisible && e->state == BOSS_BE_STATE_STEALTH_IN && e->frame >= 10) {
        return;
    }
    
    switch (e->state) {
        case BOSS_BE_STATE_WALK:
            bmp = e->walk_frames[e->frame];
            break;
        case BOSS_BE_STATE_ATTACK:
            bmp = e->attack_frames[e->frame];
            break;
        case BOSS_BE_STATE_CAST:
            bmp = e->cast_frames[e->frame];
            break;
        case BOSS_BE_STATE_SPELL:
            bmp = e->spell_frames[e->frame];
            if (bmp && e->cast_completed) {
                int flags = e->dir ? ALLEGRO_FLIP_HORIZONTAL : 0;
                float spell_draw_x = e->target_x;

                if (e->dir) {
                    spell_draw_x += al_get_bitmap_width(bmp) / 2;
                } else {
                    spell_draw_x -= al_get_bitmap_width(bmp) / 2;
                }

                al_draw_scaled_bitmap(bmp, 0, 0,
                    al_get_bitmap_width(bmp), al_get_bitmap_height(bmp),
                    spell_draw_x, e->target_y, 
                    al_get_bitmap_width(bmp) * 2, 
                    al_get_bitmap_height(bmp) * 2, flags);
            }
            bmp = e->walk_frames[0];
            break;
        case BOSS_BE_STATE_HURT:
            bmp = e->hurt_frames[e->frame];
            break;
        case BOSS_BE_STATE_STEALTH_IN:
        case BOSS_BE_STATE_STEALTH_OUT:
            bmp = e->death_frames[e->frame];
            break;
        case BOSS_BE_STATE_DIE:
            if (e->die_bitmap) {
                int frame_width = al_get_bitmap_width(e->die_bitmap) / 14;
                int frame_height = al_get_bitmap_height(e->die_bitmap);
                int flags = e->dir ? ALLEGRO_FLIP_HORIZONTAL : 0;
                float die_draw_x = e->x;

                if (e->dir) {
                    die_draw_x -= frame_width / 2;
                } else {
                    die_draw_x += frame_width / 2;
                }
                
                al_draw_scaled_bitmap(e->die_bitmap, 
                    e->frame * frame_width, 0, frame_width, frame_height,
                    die_draw_x, e->y, e->width, e->height, flags);
            }
            return; 
    }
    
    if (bmp) {
        int flags = e->dir ? ALLEGRO_FLIP_HORIZONTAL : 0;

        if (e->dir) {
            draw_x += al_get_bitmap_width(bmp) / 4;
        } else {
            draw_x -= al_get_bitmap_width(bmp) / 4;
        }
        
        al_draw_scaled_bitmap(bmp, 0, 0,
            al_get_bitmap_width(bmp), al_get_bitmap_height(bmp),
            draw_x, e->y, e->width, e->height, flags);

        // al_draw_scaled_bitmap(
        //     bmp, 0, 0,
        //     al_get_bitmap_width(bmp), al_get_bitmap_height(bmp),
        //     e->x - Camera.x, e->y - Camera.y, // 只在這裡減 Camera
        //     e->width, e->height, flags
        // );
    }
    
}

void BossEnemy_interact(Elements *self) {
    BossEnemy *e = self->pDerivedObj;
    double now = al_get_time();

    if (e->is_invisible || (now - e->last_damage_time < e->invincible_time)) {
        return;
    }

    if (e->state == BOSS_BE_STATE_DIE) {
        return;
    }
    
    ElementVec all = _Get_all_elements(scene);
    for (int i = 0; i < all.len; i++) {
        Elements *f = all.arr[i];
        if (f->label == Projectile_L) {
            Projectile *p = f->pDerivedObj;
            if (p->hitbox && e->hitbox && e->hitbox->overlap(e->hitbox, p->hitbox)) {
                e->hp--;

                if (e->state == BOSS_BE_STATE_WALK) {
                    e->state = BOSS_BE_STATE_HURT;
                    e->frame = 0;
                    e->last_frame_time = now;
                }
                e->last_damage_time = now;
                f->dele = true;
                
                printf("Boss hit by projectile! HP: %d (total attacks: %d)\n", e->hp, e->total_attack_count);
                
                if (e->hp <= 0) {
                    e->state = BOSS_BE_STATE_DIE;
                    e->frame = 0;
                    e->last_frame_time = now;
                    printf("Boss defeated!\n");
                }
                break;
            }
        }
    }
}

void BossEnemy_destroy(Elements *self) {
    BossEnemy *e = self->pDerivedObj;

    for (int i = 0; i < 8; i++) {
        if (e->walk_frames[i]) al_destroy_bitmap(e->walk_frames[i]);
    }
    for (int i = 0; i < 10; i++) {
        if (e->attack_frames[i]) al_destroy_bitmap(e->attack_frames[i]);
        if (e->death_frames[i]) al_destroy_bitmap(e->death_frames[i]);
    }
    for (int i = 0; i < 9; i++) {
        if (e->cast_frames[i]) al_destroy_bitmap(e->cast_frames[i]);
    }
    for (int i = 0; i < 16; i++) {
        if (e->spell_frames[i]) al_destroy_bitmap(e->spell_frames[i]);
    }
    for (int i = 0; i < 3; i++) {
        if (e->hurt_frames[i]) al_destroy_bitmap(e->hurt_frames[i]);
    }
    
    if (e->die_bitmap) {
        al_destroy_bitmap(e->die_bitmap);
    }
    
    if (e->hitbox) {
        free(e->hitbox);
    }
    free(e);
    free(self);
}