#include "TracingBullet.h"
#include "../element/character.h"
#include "../element/tree.h"
#include "../shapes/Circle.h"
#include "../scene/gamescene.h"
#include "../scene/sceneManager.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include "../element/elementLabel.h"

Elements *New_TracingBullet(int label, float x, float y, float vx, float vy) {
    TracingBullet *bullet = malloc(sizeof(TracingBullet));
    Elements *obj = New_Elements(label);

    bullet->img = al_load_bitmap("assets/image/Tracing-Bullet.png");
    if (bullet->img) {
        bullet->total_frames = 8;
        int original_width = al_get_bitmap_width(bullet->img) / 4; 
        int original_height = al_get_bitmap_height(bullet->img) / 2; 
        float scale = 2.0f;  // 可以調整這個數值來改變大小
        bullet->width = original_width * scale;
        bullet->height = original_height * scale;
    } else {
        printf("Warning: Could not load Tracing-Bullet.png\n");
        bullet->width = 32;
        bullet->height = 32;
        bullet->total_frames = 1;
    }
    
    bullet->x = x - bullet->width / 2; 
    bullet->y = y - bullet->height / 2;
    bullet->vx = vx;
    bullet->vy = vy;

    bullet->trace_start_time = al_get_time();
    bullet->trace_duration = 1.5; // 追蹤時間在這裡調整!!
    bullet->trace_strength = 0.8; // 追蹤強度
    bullet->is_tracing = true;
    
    bullet->frame = 0;
    bullet->last_frame_time = al_get_time();
    bullet->frame_duration = 0.5;

    bullet->hitbox = New_Circle(bullet->x + bullet->width / 2,
                               bullet->y + bullet->height / 2,
                               min(bullet->width, bullet->height) / 2);

    obj->inter_obj[obj->inter_len++] = Character_L;
    obj->inter_obj[obj->inter_len++] = Tree_L;
    obj->inter_obj[obj->inter_len++] = Floor_L;

    obj->pDerivedObj = bullet;
    obj->Update = TracingBullet_update;
    obj->Interact = TracingBullet_interact;
    obj->Draw = TracingBullet_draw;
    obj->Destroy = TracingBullet_destroy;
    
    printf("TracingBullet created at (%.1f, %.1f) with velocity (%.1f, %.1f)\n", 
           bullet->x, bullet->y, bullet->vx, bullet->vy);
    
    return obj;
}

static void update_animation(TracingBullet *bullet) {
    double now = al_get_time();
    if (now - bullet->last_frame_time >= bullet->frame_duration) {
        bullet->frame = (bullet->frame + 1) % bullet->total_frames;
        bullet->last_frame_time = now;
    }
}

void TracingBullet_update(Elements *self) {
    TracingBullet *bullet = (TracingBullet *)(self->pDerivedObj);
    
    update_animation(bullet);
    
    double now = al_get_time();

    if (bullet->is_tracing && (now - bullet->trace_start_time) < bullet->trace_duration) {
        ElementVec all = _Get_all_elements(scene);
        Elements *player = NULL;
        for (int i = 0; i < all.len; i++) {
            if (all.arr[i]->label == Character_L) { 
                player = all.arr[i]; 
                break; 
            }
        }
        
        if (player) {
            Character *c = player->pDerivedObj;
            float target_x = c->x + c->width / 2;
            float target_y = c->y + c->height / 2;
            
            float dx = target_x - (bullet->x + bullet->width / 2);
            float dy = target_y - (bullet->y + bullet->height / 2);
            float dist = sqrt(dx*dx + dy*dy);
            
            if (dist > 0) {
                float target_vx = (dx / dist) * 3.0; 
                float target_vy = (dy / dist) * 3.0;

                float lerp = bullet->trace_strength * 0.1; 
                bullet->vx = bullet->vx * (1.0 - lerp) + target_vx * lerp;
                bullet->vy = bullet->vy * (1.0 - lerp) + target_vy * lerp;

                float current_speed = sqrt(bullet->vx*bullet->vx + bullet->vy*bullet->vy);
                if (current_speed > 4.0) {
                    bullet->vx = (bullet->vx / current_speed) * 4.0;
                    bullet->vy = (bullet->vy / current_speed) * 4.0;
                }
            }
        }
    } else {
        bullet->is_tracing = false;
    }
    
    _TracingBullet_update_position(self, bullet->vx, bullet->vy);
}

void _TracingBullet_update_position(Elements *self, float dx, float dy) {
    TracingBullet *bullet = (TracingBullet *)(self->pDerivedObj);
    bullet->x += dx;
    bullet->y += dy;
    Shape *hitbox = bullet->hitbox;
    hitbox->update_center_x(hitbox, dx);
    hitbox->update_center_y(hitbox, dy);
}

void TracingBullet_interact(Elements *self) {
    for (int j = 0; j < self->inter_len; j++) {
        int inter_label = self->inter_obj[j];
        ElementVec labelEle = _Get_label_elements(scene, inter_label);
        for (int i = 0; i < labelEle.len; i++) {
            if (inter_label == Character_L) {
                _TracingBullet_interact_Character(self, labelEle.arr[i]);
            } else if (inter_label == Tree_L) {
                _TracingBullet_interact_Tree(self, labelEle.arr[i]);
            } else if (inter_label == Floor_L) {
                _TracingBullet_interact_Floor(self, labelEle.arr[i]);
            }
        }
    }
}

void _TracingBullet_interact_Character(Elements *self, Elements *tar) {
    TracingBullet *bullet = (TracingBullet *)(self->pDerivedObj);
    Character *character = (Character *)(tar->pDerivedObj);
    
    if (character->hitbox->overlap(character->hitbox, bullet->hitbox)) {
        Character_take_damage(tar, 3); // 子彈傷害在這裡調整!!
        printf("Player hit by tracing bullet! HP: %d\n", character->hp);
        self->dele = true; 
    }
}

void _TracingBullet_interact_Tree(Elements *self, Elements *tar) {
    TracingBullet *bullet = (TracingBullet *)(self->pDerivedObj);
    Tree *tree = (Tree *)(tar->pDerivedObj);
    
    if (tree->hitbox->overlap(tree->hitbox, bullet->hitbox)) {
        self->dele = true; 
    }
}

void _TracingBullet_interact_Floor(Elements *self, Elements *tar) {
    TracingBullet *bullet = (TracingBullet *)(self->pDerivedObj);

    if (bullet->x < -bullet->width || bullet->x > WIDTH + bullet->width ||
        bullet->y < -bullet->height || bullet->y > HEIGHT + bullet->height) {
        self->dele = true;
        printf("TracingBullet destroyed (out of bounds)\n");
    }
}

void TracingBullet_draw(Elements *self) {
    TracingBullet *bullet = (TracingBullet *)(self->pDerivedObj);
    
    if (bullet->img) {
        int frame_x = bullet->frame % 4; 
        int frame_y = bullet->frame / 4; 
        
        int original_width = al_get_bitmap_width(bullet->img) / 4;
        int original_height = al_get_bitmap_height(bullet->img) / 2;
        
        int src_x = frame_x * original_width;
        int src_y = frame_y * original_height;

        int flags = (bullet->vx < 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;
        
        al_draw_scaled_bitmap(bullet->img, 
                             src_x, src_y, 
                             original_width, original_height,  
                             bullet->x, bullet->y,             
                             bullet->width, bullet->height,    
                             flags);
    }
    else {
        ALLEGRO_COLOR color = bullet->is_tracing ? 
            al_map_rgb(255, 255, 0) : al_map_rgb(255, 100, 100);
        al_draw_filled_circle(bullet->x + bullet->width/2, 
                             bullet->y + bullet->height/2, 
                             bullet->width/2, 
                             color);
    }
}

void TracingBullet_destroy(Elements *self) {
    TracingBullet *bullet = (TracingBullet *)(self->pDerivedObj);
    if (bullet->img) {
        al_destroy_bitmap(bullet->img);
    }
    free(bullet->hitbox);
    free(bullet);
    free(self);
}