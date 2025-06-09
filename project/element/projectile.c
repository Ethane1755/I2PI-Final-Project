#include "projectile.h"
#include "tree.h"
#include "../shapes/Circle.h"
#include "../scene/gamescene.h" // for element label
#include "../scene/sceneManager.h" // for scene variable
#include "../enemy/BossEnemy.h"
#include "../enemy/BasicEnemy.h"
#include "../enemy/BulletEnemy.h"
#include "../enemy/TraceEnemy.h"
#include "elementLabel.h"

#include <math.h> // 追蹤敵人需要

/*
   [Projectile function]
*/
ElementVec _Get_all_enemies(Scene* scene) {
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

Elements* New_Projectile(int label, int x, int y, int v) {
    Projectile* pDerivedObj = (Projectile*)malloc(sizeof(Projectile));
    Elements* pObj = New_Elements(label);
    
    // Find closest enemy at creation time
    ElementVec enemies = _Get_all_enemies(scene);
    float tx = -1, ty = -1, min_dist = 1e9;
    float px = x;
    float py = y;

    for (int i = 0; i < enemies.len; i++) {
        Elements* e = enemies.arr[i];
        
        // Skip if it's an invisible boss
        if (e->label == BossEnemy_L) {
            BossEnemy* boss = (BossEnemy*)(e->pDerivedObj);
            if (boss->is_invisible) continue;
        }

        float ex, ey;
        if (e->label == BossEnemy_L) {
            BossEnemy* enemy = (BossEnemy*)(e->pDerivedObj);
            ex = enemy->x + enemy->width / 2.0f;
            ey = enemy->y + enemy->height / 3.0f * 2.0f;
        } else if (e->label == BulletEnemy_L) {
            BulletEnemy* enemy = (BulletEnemy*)(e->pDerivedObj);
            ex = enemy->x + enemy->width / 2.0f;
            ey = enemy->y + enemy->height / 3.0f * 2.0f;
        } else if (e->label == TraceEnemy_L) {
            TraceEnemy* enemy = (TraceEnemy*)(e->pDerivedObj);
            ex = enemy->x + enemy->width / 2.0f;
            ey = enemy->y + enemy->height / 3.0f * 2.0f;
        } else {
            BasicEnemy* enemy = (BasicEnemy*)(e->pDerivedObj);
            ex = enemy->x + enemy->width / 2.0f;
            ey = enemy->y + enemy->height / 3.0f * 2.0f;
        }

        float dx = ex - px;
        float dy = ey - py;
        float dist = sqrt(dx * dx + dy * dy);
        
        if (dist < min_dist) {
            min_dist = dist;
            tx = ex;
            ty = ey;
        }
    }

    // Set initial direction based on target
    float dx = 0, dy = -v; // Default direction (up)
    float angle = -ALLEGRO_PI/2; // Default angle (up)
    
    if (min_dist < 1e9) {
        // Calculate direction to target
        dx = tx - px;
        dy = ty - py;
        float len = sqrt(dx * dx + dy * dy);
        if (len > 0) {
            float speed = abs(v);
            dx = (dx / len) * speed;
            dy = (dy / len) * speed;
            angle = atan2(dy, dx);
        }
    }

    // setting derived object member
    pDerivedObj->img = al_load_bitmap("assets/image/projectile.png");
    pDerivedObj->width = al_get_bitmap_width(pDerivedObj->img);
    pDerivedObj->height = al_get_bitmap_height(pDerivedObj->img);
    pDerivedObj->x = x;
    pDerivedObj->y = y;
    pDerivedObj->v = v;
    // Set the projectile's direction and angle
    pDerivedObj->last_dx = dx;
    pDerivedObj->last_dy = dy;
    pDerivedObj->angle = angle;
    pDerivedObj->hitbox = New_Circle(pDerivedObj->x + pDerivedObj->width / 2,
        pDerivedObj->y + pDerivedObj->height / 2,
        min(pDerivedObj->width, pDerivedObj->height) / 2);
    // setting the interact object
    pObj->inter_obj[pObj->inter_len++] = Floor_L;
    pObj->inter_obj[pObj->inter_len++] = BasicEnemy_L;
    pObj->inter_obj[pObj->inter_len++] = BulletEnemy_L;
    pObj->inter_obj[pObj->inter_len++] = TraceEnemy_L;
    pObj->inter_obj[pObj->inter_len++] = BossEnemy_L;
    // setting derived object function
    pObj->pDerivedObj = pDerivedObj;
    pObj->Update = Projectile_update;
    pObj->Interact = Projectile_interact;
    pObj->Draw = Projectile_draw;
    pObj->Destroy = Projectile_destory;

    return pObj;
}

void _Projectile_update_position(Elements* self, float dx, float dy) {
    Projectile* Obj = (Projectile*)(self->pDerivedObj);
    Obj->x += dx;
    Obj->y += dy;
    if (Obj->hitbox) {
        Obj->hitbox->update_center_x(Obj->hitbox, dx);
        Obj->hitbox->update_center_y(Obj->hitbox, dy);
    }
}

void Projectile_update(Elements* self) {
    Projectile* Obj = ((Projectile*)(self->pDerivedObj));
    // Simply move in the initial direction without changing it
    _Projectile_update_position(self, Obj->last_dx, Obj->last_dy);
}

void Projectile_interact(Elements* self) {
    for (int j = 0; j < self->inter_len; j++) {
        int inter_label = self->inter_obj[j];
        ElementVec labelEle = _Get_label_elements(scene, inter_label);
        for (int i = 0; i < labelEle.len; i++) {
            if (inter_label == Floor_L) {
                _Projectile_interact_Floor(self, labelEle.arr[i]);
            } else if (inter_label == BasicEnemy_L || 
                      inter_label == BulletEnemy_L || 
                      inter_label == TraceEnemy_L ||
                      inter_label == BossEnemy_L) {
                _Projectile_interact_Enemy(self, labelEle.arr[i]);
            }
        }
    }
}

void _Projectile_interact_Enemy(Elements* self, Elements* tar) {
    Projectile* bullet = (Projectile*)(self->pDerivedObj);
    float bullet_center_x = bullet->x + bullet->width / 2.0f;
    float bullet_center_y = bullet->y + bullet->height / 2.0f;
    float enemy_center_x, enemy_center_y;
    float hit_threshold;

    // Check enemy state before interaction
    if (tar->label == BossEnemy_L) {
        BossEnemy* boss = (BossEnemy*)tar->pDerivedObj;
        // Skip if boss is invisible, dead/dying, or has no health
        if (boss->is_invisible || boss->state == BOSS_BE_STATE_DIE || boss->hp <= 0) {
            return;
        }
        enemy_center_x = boss->x + boss->width / 2.0f;
        enemy_center_y = boss->y + boss->height / 2.0f;
        hit_threshold = boss->width / 4.0f;
    } else {
        BasicEnemy* enemy = (BasicEnemy*)tar->pDerivedObj;
        // Skip if enemy is dead or has no health
        if (enemy->hp <= 0) {
            return;
        }
        enemy_center_x = enemy->x + enemy->width / 2.0f;
        enemy_center_y = enemy->y + enemy->height / 2.0f;
        hit_threshold = enemy->width / 4.0f;
    }
    
    // Calculate distance to enemy center
    float dx = enemy_center_x - bullet_center_x;
    float dy = enemy_center_y - bullet_center_y;
    float dist = sqrt(dx*dx + dy*dy);
    
    // Check if bullet hit the enemy
    if (dist <= hit_threshold) {
        // Calculate knockback direction
        if (dist > 0) {
            dx = (dx/dist) * 30;
            dy = (dy/dist) * 30;
        }
        
        // Apply damage based on enemy type
        if (tar->label == BossEnemy_L) {
            BossEnemy_interact(tar);
        } else if (tar->label == BulletEnemy_L) {
            BulletEnemy_take_damage(tar, 1, dx, dy);
        } else if (tar->label == TraceEnemy_L) {
            TraceEnemy_take_damage(tar, 1, dx, dy);
        } else if (tar->label == BasicEnemy_L) {
            BasicEnemy_take_damage(tar, 1, dx, dy);
        }
        
        // Destroy the projectile on hit
        self->dele = true;
    }
}

void _Projectile_interact_Floor(Elements* self, Elements* tar) {
    Projectile* Obj = ((Projectile*)(self->pDerivedObj));
    // Delete projectile when it hits any wall
    if (Obj->x < 0 || Obj->x > WIDTH || Obj->y < 0 || Obj->y > HEIGHT) {
        printf("Projectile hit wall at (%.1f,%.1f)\n", Obj->x, Obj->y);
        self->dele = true;
    }
}

void _Projectile_interact_Tree(Elements* self, Elements* tar)
{
    Projectile* Obj = ((Projectile*)(self->pDerivedObj));
    Tree* tree = ((Tree*)(tar->pDerivedObj));
    if (tree->hitbox->overlap(tree->hitbox, Obj->hitbox))
    {
        self->dele = true;
    }
}

void Projectile_draw(Elements* self)
{
    Projectile* Obj = ((Projectile*)(self->pDerivedObj));
    float cx = Obj->width / 2.0;
    float cy = Obj->height / 2.0;
    int flip_flag = 0;
    if (Obj->v < 0) {
        flip_flag = ALLEGRO_FLIP_HORIZONTAL;
    }
    al_draw_rotated_bitmap(
        Obj->img,
        cx, cy,
        Obj->x + cx, Obj->y + cy,
        Obj->angle,
        flip_flag
    );
}

void Projectile_destory(Elements* self)
{
    Projectile* Obj = ((Projectile*)(self->pDerivedObj));
    al_destroy_bitmap(Obj->img);
    free(Obj->hitbox);
    free(Obj);
    free(self);
}