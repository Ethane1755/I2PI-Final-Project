#include "TraceEnemy.h"
#include "TracingBullet.h"
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

Elements *New_TraceEnemy(int label, int x, int y) {
    TraceEnemy *enemy = malloc(sizeof(TraceEnemy));
    Elements *obj = New_Elements(label);

    enemy->move_sheet = al_load_bitmap("assets/image/TraceEnemy-Move.png");
    enemy->hit_sheet = al_load_bitmap("assets/image/TraceEnemy-Hit.png");
    enemy->attack_sheet = al_load_bitmap("assets/image/TraceEnemy-Attack.png");
    enemy->die_sheet = al_load_bitmap("assets/image/TraceEnemy-Die.png");

    enemy->move_frames = 8;
    enemy->hit_frames = 5;
    enemy->attack_frames = 17;
    enemy->die_frames = 5;
    enemy->frame = 0;
    enemy->state = TE_STATE_MOVE;
    enemy->last_frame_time = al_get_time();
    enemy->frame_duration = 0.1; 

    enemy->width = (al_get_bitmap_width(enemy->move_sheet) / enemy->move_frames) * 2;
    enemy->height = al_get_bitmap_height(enemy->move_sheet) * 2;
    enemy->x = x;
    enemy->y = y;
    enemy->speed = 1;
    enemy->dir = false;
    enemy->hp = 10;  // TraceEnemy HP
    enemy->damage = 1;
    enemy->last_damage_time = 0;

    enemy->last_shoot_time = 0;
    enemy->shoot_interval = 3.0; // 射擊間隔時間在這裡調整!!
    enemy->shoot_range = 250.0; // 射擊範圍在這裡調整!!

    enemy->hitbox = New_Rectangle(x, y, x + enemy->width, y + enemy->height);

    obj->pDerivedObj = enemy;
    obj->inter_obj[0] = Projectile_L; 
    obj->inter_len = 1;

    obj->Draw = TraceEnemy_draw;
    obj->Update = TraceEnemy_update;
    obj->Interact = TraceEnemy_interact;
    obj->Destroy = TraceEnemy_destroy;
    return obj;
}

static void update_animation(TraceEnemy *e, Elements *self) {
    double now = al_get_time();
    if (now - e->last_frame_time >= e->frame_duration) {
        e->frame++;
        int maxf = 1;
        switch (e->state) {
            case TE_STATE_MOVE: maxf = e->move_frames; break;
            case TE_STATE_HIT: maxf = e->hit_frames; break;
            case TE_STATE_ATTACK: maxf = e->attack_frames; break;
            case TE_STATE_DIE: maxf = e->die_frames; break;
        }
        if (e->frame >= maxf) {
            if (e->state == TE_STATE_HIT || e->state == TE_STATE_ATTACK) {
                e->state = TE_STATE_MOVE;
                e->frame = 0;
            } else if (e->state == TE_STATE_DIE) {
                self->dele = true;
                return;
            } else {
                e->frame %= maxf;
            }
        }
        e->last_frame_time = now;
    }
}

void TraceEnemy_update(Elements *self) {
    TraceEnemy *e = self->pDerivedObj;

    if (e->state == TE_STATE_DIE) {
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
            TraceEnemy_shoot(self, c->x + c->width/2, c->y + c->height/2);
            e->last_shoot_time = now;
            e->state = TE_STATE_ATTACK;
            e->frame = 0;
        }

        if (dist > e->shoot_range - 10) { 
            // 太遠了，靠近一點
            e->dir = dx < 0; 
            float mvx = (dx/dist) * e->speed;
            float mvy = (dy/dist) * e->speed;
            _TraceEnemy_update_position(self, mvx, mvy);
        } else if (dist < e->shoot_range - 50) {
            // 太近了，後退一點
            e->dir = dx > 0; 
            float mvx = -(dx/dist) * e->speed * 0.5;
            float mvy = -(dy/dist) * e->speed * 0.5;
            _TraceEnemy_update_position(self, mvx, mvy);
        } else {
            e->dir = dx < 0; 
        }
    }
    update_animation(e, self);
}

void TraceEnemy_interact(Elements *self) {
    TraceEnemy *e = self->pDerivedObj;

    if (e->state == TE_STATE_DIE) {
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
                TraceEnemy_take_damage(self, 1, kdx, kdy);
                ele->dele = true;
            }
        }
    }
}

void TraceEnemy_draw(Elements *self) {
    TraceEnemy *e = self->pDerivedObj;
    ALLEGRO_BITMAP *sheet = e->move_sheet;
    int maxf = e->move_frames;
    
    switch (e->state) {
        case TE_STATE_MOVE: sheet = e->move_sheet; maxf = e->move_frames; break;
        case TE_STATE_HIT: sheet = e->hit_sheet; maxf = e->hit_frames; break;
        case TE_STATE_ATTACK: sheet = e->attack_sheet; maxf = e->attack_frames; break;
        case TE_STATE_DIE: sheet = e->die_sheet; maxf = e->die_frames; break;
    }

    int fw, fh, src_x, src_y = 0;
    
    fw = al_get_bitmap_width(sheet) / maxf;
    fh = al_get_bitmap_height(sheet);
    src_x = e->frame * fw;
    src_y = 0;

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
    // al_draw_circle(e->x + e->width/2, e->y + e->height/2, e->shoot_range, al_map_rgb(0, 255, 0), 1);
}

void TraceEnemy_shoot(Elements *self, float target_x, float target_y) {
    TraceEnemy *e = self->pDerivedObj;

    float start_x = e->x + e->width / 2;
    float start_y = e->y + e->height / 2;

    float dx = target_x - start_x;
    float dy = target_y - start_y;
    float dist = sqrt(dx*dx + dy*dy);
    
    if (dist > 0) {
        float bullet_speed = 2.0; // 子彈速度在這裡調整!!
 
        float vel_x = (dx/dist) * bullet_speed;
        float vel_y = (dy/dist) * bullet_speed;
    
        Elements *bullet = New_TracingBullet(TracingBullet_L, start_x, start_y, vel_x, vel_y);
     
        _Register_elements(scene, bullet);
        
        printf("TraceEnemy shoots tracing bullet! Position: (%.1f, %.1f) -> (%.1f, %.1f), Velocity: (%.1f, %.1f)\n", 
               start_x, start_y, target_x, target_y, vel_x, vel_y);
    }
}

void TraceEnemy_take_damage(Elements *self, int dmg, float kx, float ky) {
    TraceEnemy *e = self->pDerivedObj;
    e->hp -= dmg;
    e->x += kx; e->y += ky;
    e->hitbox->update_center_x(e->hitbox, kx);
    e->hitbox->update_center_y(e->hitbox, ky);
    e->state = TE_STATE_HIT;
    e->frame = 0;
    e->last_damage_time = al_get_time();
    printf("TraceEnemy damaged! HP: %d\n", e->hp);
    if (e->hp <= 0) {
        e->state = TE_STATE_DIE;
        e->frame = 0;
        e->death_start_time = al_get_time(); 
        printf("TraceEnemy defeated!\n");
    }
}

void _TraceEnemy_update_position(Elements *self, float dx, float dy) {
    TraceEnemy *e = self->pDerivedObj;
    e->x += dx; e->y += dy;
    e->hitbox->update(e->hitbox, e->x, e->y, e->x + e->width, e->y + e->height);
}

void TraceEnemy_destroy(Elements *self) {
    TraceEnemy *e = self->pDerivedObj;
    if (e->move_sheet) al_destroy_bitmap(e->move_sheet);
    if (e->hit_sheet) al_destroy_bitmap(e->hit_sheet);
    if (e->attack_sheet) al_destroy_bitmap(e->attack_sheet);
    if (e->die_sheet) al_destroy_bitmap(e->die_sheet);
    free(e->hitbox);
    free(e);
    free(self);
}