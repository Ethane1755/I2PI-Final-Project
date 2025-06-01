#include "projectile.h"
#include "tree.h"
#include "../shapes/Circle.h"
#include "../scene/gamescene.h" // for element label
#include "../scene/sceneManager.h" // for scene variable
#include "../enemy/BossEnemy.h"
#include "../enemy/BasicEnemy.h"
#include "../enemy/BulletEnemy.h"
#include "../enemy/TraceEnemy.h"

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

Elements* New_Projectile(int label, int x, int y, int v)
{
    Projectile* pDerivedObj = (Projectile*)malloc(sizeof(Projectile));
    Elements* pObj = New_Elements(label);
    // setting derived object member
    pDerivedObj->img = al_load_bitmap("assets/image/projectile.png");
    pDerivedObj->width = al_get_bitmap_width(pDerivedObj->img);
    pDerivedObj->height = al_get_bitmap_height(pDerivedObj->img);
    pDerivedObj->x = x;
    pDerivedObj->y = y;
    pDerivedObj->v = v;
    pDerivedObj->hitbox = New_Circle(pDerivedObj->x + pDerivedObj->width / 2,
        pDerivedObj->y + pDerivedObj->height / 2,
        min(pDerivedObj->width, pDerivedObj->height) / 2);
    // setting the interact object
    //pObj->inter_obj[pObj->inter_len++] = Tree_L;
    pObj->inter_obj[pObj->inter_len++] = Floor_L;
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

void Projectile_update(Elements* self)
{
    Projectile* Obj = ((Projectile*)(self->pDerivedObj));
    ElementVec enemies = _Get_all_enemies(scene);
    float tx = -1, ty = -1, min_dist = 1e9;

    for (int i = 0; i < enemies.len; i++) {
        Elements* e = enemies.arr[i];
        if (enemies.arr[i]->label == BossEnemy_L) {
            BossEnemy* enemyObj = (BossEnemy*)(e->pDerivedObj);
            if (enemyObj->is_invisible) return;
            float ex = enemyObj->x + enemyObj->width / 2.0f;
            float ey = enemyObj->y + enemyObj->height / 3.0f * 2.0f;
            float px = Obj->x + Obj->width / 2.0f;
            float py = Obj->y + Obj->height / 2.0f;
            float dx = ex - px;
            float dy = ey - py;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < min_dist) {
                min_dist = dist;
                tx = ex;
                ty = ey;
            }
        }
        else if (enemies.arr[i]->label == BulletEnemy_L) {
            BulletEnemy* enemyObj = (BulletEnemy*)(e->pDerivedObj);
            float ex = enemyObj->x + enemyObj->width / 2.0f;
            float ey = enemyObj->y + enemyObj->height / 3.0f * 2.0f;
            float px = Obj->x + Obj->width / 2.0f;
            float py = Obj->y + Obj->height / 2.0f;
            float dx = ex - px;
            float dy = ey - py;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < min_dist) {
                min_dist = dist;
                tx = ex;
                ty = ey;
            }
        }
        else if (enemies.arr[i]->label == TraceEnemy_L) {
            TraceEnemy* enemyObj = (TraceEnemy*)(e->pDerivedObj);
            float ex = enemyObj->x + enemyObj->width / 2.0f;
            float ey = enemyObj->y + enemyObj->height / 3.0f * 2.0f;
            float px = Obj->x + Obj->width / 2.0f;
            float py = Obj->y + Obj->height / 2.0f;
            float dx = ex - px;
            float dy = ey - py;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < min_dist) {
                min_dist = dist;
                tx = ex;
                ty = ey;
            }
        }
        else {
            BasicEnemy* enemyObj = (BasicEnemy*)(e->pDerivedObj);
            float ex = enemyObj->x + enemyObj->width / 2.0f;
            float ey = enemyObj->y + enemyObj->height / 3.0f * 2.0f;
            float px = Obj->x + Obj->width / 2.0f;
            float py = Obj->y + Obj->height / 2.0f;
            float dx = ex - px;
            float dy = ey - py;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < min_dist) {
                min_dist = dist;
                tx = ex;
                ty = ey;
            }
        }
    }

    if (min_dist < 1e9) {
        float px = Obj->x + Obj->width / 2.0f;
        float py = Obj->y + Obj->height / 2.0f;
        float dx = tx - px;
        float dy = ty - py;
        float len = sqrt(dx * dx + dy * dy);
        if (len > 0) {
            float speed = abs(Obj->v);
            dx = dx / len * speed;
            dy = dy / len * speed;
            Obj->angle = atan2(dy, dx);
            _Projectile_update_position(self, dx, dy);
            return;
        }
        else {
            Obj->angle = -ALLEGRO_PI / 2; // 預設向上
            _Projectile_update_position(self, 0, -Obj->v);
        }
    }
    Obj->angle = -ALLEGRO_PI / 2;
    _Projectile_update_position(self, 0, -Obj->v);
}

void Projectile_interact(Elements* self)
{
    for (int j = 0; j < self->inter_len; j++)
    {
        int inter_label = self->inter_obj[j];
        ElementVec labelEle = _Get_label_elements(scene, inter_label);
        for (int i = 0; i < labelEle.len; i++)
        {
            if (inter_label == Floor_L)
            {
                _Projectile_interact_Floor(self, labelEle.arr[i]);
            }
            /*else if (inter_label == Tree_L)
            {
                _Projectile_interact_Tree(self, labelEle.arr[i]);
            }*/
        }
    }
}

void _Projectile_interact_Floor(Elements* self, Elements* tar)
{
    Projectile* Obj = ((Projectile*)(self->pDerivedObj));
    if (Obj->x < 0 - Obj->width)
        self->dele = true;
    else if (Obj->x > WIDTH + Obj->width)
        self->dele = true;
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