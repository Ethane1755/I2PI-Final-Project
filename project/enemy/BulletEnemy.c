#include "BulletEnemy.h"
#include "EnemyBullet.h"
#include "../element/character.h"
#include "../element/projectile.h"
#include "../shapes/Rectangle.h"
#include "../scene/sceneManager.h"
#include "../scene/gamescene.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include "../element/elementLabel.h"

Elements *New_BulletEnemy(int label, int x, int y) {
    BulletEnemy *enemy = malloc(sizeof(BulletEnemy));
    Elements *obj = New_Elements(label);

    enemy->run_sheet = al_load_bitmap("assets/image/BulletEnemy_Move.png");
    enemy->hit_sheet = al_load_bitmap("assets/image/BulletEnemy_Hit.png");
    enemy->attack_sheet = al_load_bitmap("assets/image/BulletEnemy_Attack.png");
    enemy->death_sheet = al_load_bitmap("assets/image/BulletEnemy_Die.png");

    enemy->run_frames = 10;
    enemy->hit_frames = 8;
    enemy->attack_frames = 9;
    enemy->death_frames = 14;
    enemy->frame = 0;
    enemy->state = BLE_STATE_RUN;
    enemy->last_frame_time = al_get_time();
    enemy->frame_duration = 0.1; 

    enemy->width = (al_get_bitmap_width(enemy->run_sheet) / enemy->run_frames) * 2;
    enemy->height = al_get_bitmap_height(enemy->run_sheet) * 2;
    enemy->x = x;
    enemy->y = y;
    enemy->speed = 1;
    enemy->dir = false;
    enemy->hp = 8;  // BulletEnemy HP - already correct!
    enemy->damage = 1;
    enemy->last_damage_time = 0;

    enemy->last_shoot_time = 0;
    enemy->shoot_interval = 2.0;
    enemy->shoot_range = 200.0; //射擊範圍在這裡調!!

    enemy->hitbox = New_Rectangle(x, y, x + enemy->width, y + enemy->height);

    obj->pDerivedObj = enemy;
    obj->inter_obj[0] = Projectile_L; 
    obj->inter_len = 1;

    obj->Draw = BulletEnemy_draw;
    obj->Update = BulletEnemy_update;
    obj->Interact = BulletEnemy_interact;
    obj->Destroy = BulletEnemy_destroy;
    return obj;
}

static void update_animation(BulletEnemy *e, Elements *self) {
    double now = al_get_time();
    if (now - e->last_frame_time >= e->frame_duration) {
        e->frame++;
        int maxf = 1;
        switch (e->state) {
            case BLE_STATE_RUN: maxf = e->run_frames; break;
            case BLE_STATE_HIT: maxf = e->hit_frames; break;
            case BLE_STATE_ATTACK: maxf = e->attack_frames; break;
            case BLE_STATE_DEATH: maxf = e->death_frames; break;
        }
        if (e->frame >= maxf) {
            if (e->state == BLE_STATE_HIT || e->state == BLE_STATE_ATTACK) {
                e->state = BLE_STATE_RUN;
                e->frame = 0;
            } else if (e->state == BLE_STATE_DEATH) {
                self->dele = true;
                return;
            } else {
                e->frame %= maxf;
            }
        }
        e->last_frame_time = now;
    }
}

void BulletEnemy_update(Elements *self) {
    BulletEnemy *e = self->pDerivedObj;

    if (e->state == BLE_STATE_DEATH) {
        update_animation(e, self);
        return;
    }
    
    ElementVec all = _Get_all_elements(scene);
    Elements *player = NULL;
    for (int i = 0; i < all.len; i++)
        if (all.arr[i]->label == Character_L) { player = all.arr[i]; break; }
    
    if (player) {
        Character *c = player->pDerivedObj;
        float dx = c->x - e->x;
        float dy = c->y - e->y;
        double dist = sqrt(dx*dx + dy*dy);
 
        double now = al_get_time();
        if (dist <= e->shoot_range && now - e->last_shoot_time >= e->shoot_interval) {
            BulletEnemy_shoot(self, c->x + c->width/2, c->y + c->height/2);
            e->last_shoot_time = now;
            e->state = BLE_STATE_ATTACK;
            e->frame = 0;
        }

        if (dist > e->shoot_range - 10) { 
            // 太遠了，靠近一點
            e->dir = dx < 0; 
            float mvx = (dx/dist) * e->speed;
            float mvy = (dy/dist) * e->speed;
            _BulletEnemy_update_position(self, mvx, mvy);
        } else if (dist < e->shoot_range - 40) {
            // 太近了，後退一點
            e->dir = dx > 0; 
            float mvx = -(dx/dist) * e->speed * 0.5;
            float mvy = -(dy/dist) * e->speed * 0.5;
            _BulletEnemy_update_position(self, mvx, mvy);
        } else {
            e->dir = dx < 0; 
        }
    }
    update_animation(e, self);
}

void BulletEnemy_interact(Elements *self) {
    BulletEnemy *e = self->pDerivedObj;

    if (e->state == BLE_STATE_DEATH) {
        return;
    }
    
    ElementVec all = _Get_all_elements(scene);
    for (int i = 0; i < all.len; i++) {
        Elements *ele = all.arr[i];
        if (ele->label == Projectile_L) {
            Projectile *p = ele->pDerivedObj;
            if (p->hitbox && e->hitbox->overlap(e->hitbox, p->hitbox)) {
                float kdx = e->x - p->x;
                float kdy = e->y - p->y;
                float d = sqrt(kdx*kdx + kdy*kdy);
                if (d > 0) {
                    kdx = (kdx/d)*30;
                    kdy = (kdy/d)*30;
                }
                BulletEnemy_take_damage(self, 1, kdx, kdy);
                ele->dele = true;
            }
        }
    }
}

void BulletEnemy_draw(Elements *self) {
    BulletEnemy *e = self->pDerivedObj;
    ALLEGRO_BITMAP *sheet = e->run_sheet;
    int maxf = e->run_frames;
    
    switch (e->state) {
        case BLE_STATE_RUN: sheet = e->run_sheet; maxf = e->run_frames; break;
        case BLE_STATE_HIT: sheet = e->hit_sheet; maxf = e->hit_frames; break;
        case BLE_STATE_ATTACK: sheet = e->attack_sheet; maxf = e->attack_frames; break;
        case BLE_STATE_DEATH: sheet = e->death_sheet; maxf = e->death_frames; break;
    }

    int fw, fh, src_x, src_y = 0;
    
    if (e->state == BLE_STATE_DEATH) {
        int total_width = al_get_bitmap_width(sheet);
        int total_height = al_get_bitmap_height(sheet);
        
        fw = total_width / 10;  
        fh = total_height / 2;  
        
        if (e->frame < 10) {
            src_x = e->frame * fw;
            src_y = 0;
        } 
        else {
            src_x = (e->frame - 10) * fw;
            src_y = fh;
        }
    } 
    else {
        fw = al_get_bitmap_width(sheet) / maxf;
        fh = al_get_bitmap_height(sheet);
        src_x = e->frame * fw;
        src_y = 0;
    }

    int flags = e->dir ? ALLEGRO_FLIP_HORIZONTAL : 0;

    float scale = 2.0; 
    al_draw_scaled_bitmap(
        sheet,
        src_x, src_y, fw, fh,          
        e->x, e->y,              
        fw * scale, fh * scale,     
        flags
    );
    
    // 射程範圍圖像化（除錯用）
    // al_draw_circle(e->x + e->width/2, e->y + e->height/2, e->shoot_range, al_map_rgb(255, 0, 0), 1);
}

void BulletEnemy_shoot(Elements *self, float target_x, float target_y) {
    BulletEnemy *e = self->pDerivedObj;

    float start_x = e->x + e->width / 2;
    float start_y = e->y + e->height / 2;

    float dx = target_x - start_x;
    float dy = target_y - start_y;
    float dist = sqrt(dx*dx + dy*dy);
    
    if (dist > 0) {
        // 子彈速度
        float bullet_speed = 3.0; 
 
        float vel_x = (dx/dist) * bullet_speed;
        float vel_y = (dy/dist) * bullet_speed;
    
        Elements *bullet = New_EnemyBullet(EnemyBullet_L, start_x, start_y, vel_x, vel_y);
     
        _Register_elements(scene, bullet);
        
        printf("BulletEnemy shoots! Position: (%.1f, %.1f) -> (%.1f, %.1f), Velocity: (%.1f, %.1f)\n", 
               start_x, start_y, target_x, target_y, vel_x, vel_y);
    }
}

void BulletEnemy_take_damage(Elements *self, int dmg, float kx, float ky) {
    BulletEnemy *e = self->pDerivedObj;
    e->hp -= dmg;
    e->x += kx; e->y += ky;
    e->hitbox->update_center_x(e->hitbox, kx);
    e->hitbox->update_center_y(e->hitbox, ky);
    e->state = BLE_STATE_HIT;
    e->frame = 0;
    e->last_damage_time = al_get_time();
    printf("BulletEnemy damaged! HP: %d\n", e->hp);
    if (e->hp <= 0) {
        e->state = BLE_STATE_DEATH;
        e->frame = 0;
        e->death_start_time = al_get_time(); 
        printf("BulletEnemy defeated!\n");
    }
}

void _BulletEnemy_update_position(Elements *self, float dx, float dy) {
    BulletEnemy *e = self->pDerivedObj;
    e->x += dx; e->y += dy;
    e->hitbox->update(e->hitbox, e->x, e->y, e->x + e->width, e->y + e->height);
}

void BulletEnemy_destroy(Elements *self) {
    BulletEnemy *e = self->pDerivedObj;
    if (e->run_sheet) al_destroy_bitmap(e->run_sheet);
    if (e->hit_sheet) al_destroy_bitmap(e->hit_sheet);
    if (e->attack_sheet) al_destroy_bitmap(e->attack_sheet);
    if (e->death_sheet) al_destroy_bitmap(e->death_sheet);
    free(e->hitbox);
    free(e);
    free(self);
}