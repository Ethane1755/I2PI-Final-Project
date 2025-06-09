#include "EnemyBullet.h"
#include "../element/character.h"
#include "../element/tree.h"
#include "../shapes/Circle.h"
#include "../scene/gamescene.h"
#include "../scene/sceneManager.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include "../element/elementLabel.h"

Elements *New_EnemyBullet(int label, float x, float y, float vx, float vy) {
    EnemyBullet *bullet = malloc(sizeof(EnemyBullet));
    Elements *obj = New_Elements(label);

    bullet->img = al_load_bitmap("assets/image/BulletEnemy_Bullet.png");
    if (bullet->img) {
        bullet->width = al_get_bitmap_width(bullet->img);
        bullet->height = al_get_bitmap_height(bullet->img);
    } else {
        printf("Warning: Could not load BulletEnemy_Bullet.png\n");
        bullet->width = 16;
        bullet->height = 16;
    }
    
    bullet->x = x - bullet->width / 2; 
    bullet->y = y - bullet->height / 2;
    bullet->vx = vx;
    bullet->vy = vy;

    bullet->hitbox = New_Circle(bullet->x + bullet->width / 2,
                               bullet->y + bullet->height / 2,
                               min(bullet->width, bullet->height) / 2);

    obj->inter_obj[obj->inter_len++] = Character_L;
    obj->inter_obj[obj->inter_len++] = Tree_L;
    obj->inter_obj[obj->inter_len++] = Floor_L;

    obj->pDerivedObj = bullet;
    obj->Update = EnemyBullet_update;
    obj->Interact = EnemyBullet_interact;
    obj->Draw = EnemyBullet_draw;
    obj->Destroy = EnemyBullet_destroy;
    
    printf("EnemyBullet created at (%.1f, %.1f) with velocity (%.1f, %.1f)\n", 
           bullet->x, bullet->y, bullet->vx, bullet->vy);
    
    return obj;
}

void EnemyBullet_update(Elements *self) {
    EnemyBullet *bullet = (EnemyBullet *)(self->pDerivedObj);
    _EnemyBullet_update_position(self, bullet->vx, bullet->vy);
}

void _EnemyBullet_update_position(Elements *self, float dx, float dy) {
    EnemyBullet *bullet = (EnemyBullet *)(self->pDerivedObj);
    bullet->x += dx;
    bullet->y += dy;
    Shape *hitbox = bullet->hitbox;
    hitbox->update_center_x(hitbox, dx);
    hitbox->update_center_y(hitbox, dy);
}

void EnemyBullet_interact(Elements *self) {
    for (int j = 0; j < self->inter_len; j++) {
        int inter_label = self->inter_obj[j];
        ElementVec labelEle = _Get_label_elements(scene, inter_label);
        for (int i = 0; i < labelEle.len; i++) {
            if (inter_label == Character_L) {
                _EnemyBullet_interact_Character(self, labelEle.arr[i]);
            } else if (inter_label == Tree_L) {
                _EnemyBullet_interact_Tree(self, labelEle.arr[i]);
            } else if (inter_label == Floor_L) {
                _EnemyBullet_interact_Floor(self, labelEle.arr[i]);
            }
        }
    }
}

void _EnemyBullet_interact_Character(Elements *self, Elements *tar) {
    EnemyBullet *bullet = (EnemyBullet *)(self->pDerivedObj);
    Character *character = (Character *)(tar->pDerivedObj);
    
    if (character->hitbox->overlap(character->hitbox, bullet->hitbox)) {
        Character_take_damage(tar, 2); // 子彈傷害在這裡調!!
        printf("Player hit by enemy bullet! HP: %d\n", character->hp);
        self->dele = true; 
    }
}

void _EnemyBullet_interact_Tree(Elements *self, Elements *tar) {
    EnemyBullet *bullet = (EnemyBullet *)(self->pDerivedObj);
    Tree *tree = (Tree *)(tar->pDerivedObj);
    
    if (tree->hitbox->overlap(tree->hitbox, bullet->hitbox)) {
        self->dele = true; 
    }
}

void _EnemyBullet_interact_Floor(Elements *self, Elements *tar) {
    EnemyBullet *bullet = (EnemyBullet *)(self->pDerivedObj);

    if (bullet->x < -bullet->width || bullet->x > WIDTH + bullet->width ||
        bullet->y < -bullet->height || bullet->y > HEIGHT + bullet->height) {
        self->dele = true;
        printf("EnemyBullet destroyed (out of bounds)\n");
    }
}

void EnemyBullet_draw(Elements *self) {
    EnemyBullet *bullet = (EnemyBullet *)(self->pDerivedObj);
    
    if (bullet->img) {
        int flags = (bullet->vx > 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;
        al_draw_bitmap(bullet->img, bullet->x, bullet->y, flags);
    } else {
        al_draw_filled_circle(bullet->x + bullet->width/2, 
                             bullet->y + bullet->height/2, 
                             bullet->width/2, 
                             al_map_rgb(255, 0, 0));
    }
}

void EnemyBullet_destroy(Elements *self) {
    EnemyBullet *bullet = (EnemyBullet *)(self->pDerivedObj);
    if (bullet->img) {
        al_destroy_bitmap(bullet->img);
    }
    free(bullet->hitbox);
    free(bullet);
    free(self);
}