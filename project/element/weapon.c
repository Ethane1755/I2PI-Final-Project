#include "weapon.h"
#include "projectile.h"
#include "../scene/gamescene.h"
#include "../algif5/algif.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <math.h>
#include "elementLabel.h"

static ALGIF_ANIMATION *weapon_gif = NULL;

void Weapon_draw(int x, int y) {
    if (!weapon_gif) {
        weapon_gif = algif_new_gif("assets/image/weapon.gif", -1);
    }
    ALLEGRO_BITMAP *frame = algif_get_bitmap(weapon_gif, al_get_time());
    if (frame) {
        al_draw_bitmap(frame, x, y, 0);
    }
}

void Weapon_draw_rotated(int x, int y, float angle) {
    if (!weapon_gif) {
        weapon_gif = algif_new_gif("assets/image/weapon.gif", -1);
    }
    ALLEGRO_BITMAP *frame = algif_get_bitmap(weapon_gif, al_get_time());
    if (frame) {
        int cx = al_get_bitmap_width(frame) / 2;
        int cy = al_get_bitmap_height(frame) / 2;
        al_draw_rotated_bitmap(frame, cx, cy, x + cx, y + cy, angle, 0);
    }
}

void Character_fire_projectile(Character *chara, int state) {
    int proj_x = chara->x + chara->width / 2;
    int proj_y = chara->y;
    int proj_speed = 10; // Increased speed for better responsiveness
    
    // Default direction is up/down based on state
    float default_angle = (state == IDLE) ? -ALLEGRO_PI/2 : ALLEGRO_PI/2;
    
    Elements *pro = New_Projectile(Projectile_L, proj_x, proj_y, proj_speed);
    if (pro) {
        _Register_elements(scene, pro);
        // 記錄最後一顆 projectile
        chara->last_proj = (Projectile *)(pro->pDerivedObj);
        
        // If no target was found, manually set the angle
        if (chara->last_proj->angle == 0) {
            chara->last_proj->angle = default_angle;
            chara->last_proj->last_dx = proj_speed * cos(default_angle);
            chara->last_proj->last_dy = proj_speed * sin(default_angle);
        }
    }
}
