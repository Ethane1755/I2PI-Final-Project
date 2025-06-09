/* BasicEnemy.c */
#include "BasicEnemy.h"
#include "../element/character.h"
#include "../element/projectile.h"
#include "../shapes/Rectangle.h"
#include "../scene/sceneManager.h"
#include "../scene/gamescene.h"
#include "../element/character.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include "../element/elementLabel.h"

Elements *New_BasicEnemy(int label, int x, int y) {
    BasicEnemy *enemy = malloc(sizeof(BasicEnemy));
    Elements *obj = New_Elements(label);

    enemy->run_sheet = al_load_bitmap("assets/image/Mushroom-Run.png");
    enemy->hit_sheet = al_load_bitmap("assets/image/Mushroom-Hit.png");
    enemy->attack_sheet = al_load_bitmap("assets/image/Mushroom-Attack.png");
    enemy->death_sheet = al_load_bitmap("assets/image/Mushroom-Die.png");

    enemy->run_frames = 8;
    enemy->hit_frames = 5;
    enemy->attack_frames = 10;
    enemy->death_frames = 15;
    enemy->frame = 0;
    enemy->state = BE_STATE_RUN;
    enemy->last_frame_time = al_get_time();
    enemy->frame_duration = 0.1; 

    enemy->width = (al_get_bitmap_width(enemy->run_sheet) / enemy->run_frames) * 2;
    enemy->height = al_get_bitmap_height(enemy->run_sheet) * 2;
    enemy->x = x;
    enemy->y = y;
    enemy->speed = 1;
    enemy->dir = false;
    enemy->hp = 5;  // Basic enemy HP
    enemy->damage = 10;  // Increased from 1 to make combat more challenging
    enemy->last_damage_time = 0;

    enemy->hitbox = New_Rectangle(x, y, x + enemy->width, y + enemy->height);

    obj->pDerivedObj = enemy;
    obj->inter_obj[0] = Character_L;
    obj->inter_obj[1] = Projectile_L;
    obj->inter_len = 2;

    obj->Draw = BasicEnemy_draw;
    obj->Update = BasicEnemy_update;
    obj->Interact = BasicEnemy_interact;
    obj->Destroy = BasicEnemy_destroy;
    return obj;
}

static void update_animation(BasicEnemy *e, Elements *self) {
    double now = al_get_time();
    if (now - e->last_frame_time >= e->frame_duration) {
        e->frame++;
        int maxf = 1;
        switch (e->state) {
            case BE_STATE_RUN: maxf = e->run_frames; break;
            case BE_STATE_HIT: maxf = e->hit_frames; break;
            case BE_STATE_ATTACK: maxf = e->attack_frames; break;
            case BE_STATE_DEATH: maxf = e->death_frames; break;
        }
        if (e->frame >= maxf) {
            if (e->state == BE_STATE_HIT || e->state == BE_STATE_ATTACK) {
                e->state = BE_STATE_RUN;
                e->frame = 0;
            } else if (e->state == BE_STATE_DEATH) {
                self->dele = true;
                return;
            } else {
                e->frame %= maxf;
            }
        }
        e->last_frame_time = now;
    }
}

void BasicEnemy_update(Elements *self) {
    BasicEnemy *e = self->pDerivedObj;

    if (e->state == BE_STATE_DEATH) {
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
        if (dist > 80) { 
            e->dir = dx > 0;
            float mvx = (dx/dist) * e->speed;
            float mvy = (dy/dist) * e->speed;
            _BasicEnemy_update_position(self, mvx, mvy);
        }
    }
    update_animation(e, self);
}

void BasicEnemy_interact(Elements *self) {
    BasicEnemy *e = self->pDerivedObj;

    if (e->state == BE_STATE_DEATH) {
        return;
    }
    
    ElementVec all = _Get_all_elements(scene);
    double now = al_get_time();
    for (int i = 0; i < all.len; i++) {
        Elements *ele = all.arr[i];
        if (ele->label == Character_L) {
            Character *c = ele->pDerivedObj;
            if (e->hitbox->overlap(e->hitbox, c->hitbox) && now - e->last_damage_time > 0.5) {
                Character_take_damage(ele, e->damage);
                e->last_damage_time = now;
                e->state = BE_STATE_ATTACK;
                e->frame = 0;
            }
        } else if (ele->label == Projectile_L) {
            Projectile *p = ele->pDerivedObj;
            if (p->hitbox && e->hitbox->overlap(e->hitbox, p->hitbox)) {
                float kdx = e->x - p->x;
                float kdy = e->y - p->y;
                float d = sqrt(kdx*kdx + kdy*kdy);
                if (d > 0) {
                    kdx = (kdx/d)*30;
                    kdy = (kdy/d)*30;
                }
                BasicEnemy_take_damage(self, 1, kdx, kdy);
                ele->dele = true;
            }
        }
    }
}

void BasicEnemy_draw(Elements *self) {
    BasicEnemy *e = self->pDerivedObj;
    ALLEGRO_BITMAP *sheet = e->run_sheet;
    int maxf = e->run_frames;
    switch (e->state) {
        case BE_STATE_RUN: sheet = e->run_sheet; maxf = e->run_frames; break;
        case BE_STATE_HIT: sheet = e->hit_sheet; maxf = e->hit_frames; break;
        case BE_STATE_ATTACK: sheet = e->attack_sheet; maxf = e->attack_frames; break;
        case BE_STATE_DEATH: sheet = e->death_sheet; maxf = e->death_frames; break;
    }

    int fw = al_get_bitmap_width(sheet) / maxf;
    int fh = al_get_bitmap_height(sheet);
    int src_x = e->frame * fw;
    int flags = e->dir ? ALLEGRO_FLIP_HORIZONTAL : 0;

    float scale = 2.0; 
    al_draw_scaled_bitmap(
        sheet,
        src_x, 0, fw, fh,          
        e->x, e->y,              
        fw * scale, fh * scale,     
        flags
    );
}


void BasicEnemy_take_damage(Elements *self, int dmg, float kx, float ky) {
    BasicEnemy *e = self->pDerivedObj;
    e->hp -= dmg;
    e->x += kx; e->y += ky;
    e->hitbox->update_center_x(e->hitbox, kx);
    e->hitbox->update_center_y(e->hitbox, ky);
    e->state = BE_STATE_HIT;
    e->frame = 0;
    e->last_damage_time = al_get_time();
    printf("Enemy damaged! HP: %d\n", e->hp);
    if (e->hp <= 0) {
        e->state = BE_STATE_DEATH;
        e->frame = 0;
        e->death_start_time = al_get_time(); 
        printf("Enemy defeated!\n");
    }
}

void _BasicEnemy_update_position(Elements *self, float dx, float dy) {
    BasicEnemy *e = self->pDerivedObj;
    e->x += dx; e->y += dy;
    e->hitbox->update(e->hitbox, e->x, e->y, e->x + e->width, e->y + e->height);
}

void BasicEnemy_destroy(Elements *self) {
    BasicEnemy *e = self->pDerivedObj;
    if (e->run_sheet) al_destroy_bitmap(e->run_sheet);
    if (e->hit_sheet) al_destroy_bitmap(e->hit_sheet);
    if (e->attack_sheet) al_destroy_bitmap(e->attack_sheet);
    if (e->death_sheet) al_destroy_bitmap(e->death_sheet);
    free(e->hitbox);
    free(e);
    free(self);
}
