#include "skill.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef SKILL_NONE
#define SKILL_NONE -1
#endif

// Only keep 4 skills
const char* skill_names[4] = {
    "HP +30%",
    "Shield +10%",
    "Speed +30%",
    "Multi Shot"
};


void SkillSystem_init(SkillSystem* sys) {
    sys->selected_skill = SKILL_NONE;
    sys->last_select_time = al_get_time();
    sys->selecting = false;
    srand(time(NULL));
}

void SkillSystem_start_select(SkillSystem* sys) {
    sys->selecting = true;
    sys->last_select_time = al_get_time();
    int pool[SKILL_COUNT];
    for (int i = 0; i < SKILL_COUNT; i++) pool[i] = i;
    for (int i = 0; i < 3; i++) {
        int remain = SKILL_COUNT - i;
        int idx = rand() % remain;
        sys->options[i] = pool[idx];
        pool[idx] = pool[remain - 1];
    }
}

void SkillSystem_update(SkillSystem* sys, Character* player) {
    double now = al_get_time();
    // 只有在沒選技能時才計時
    if (!sys->selecting && now - sys->last_select_time > 5) {
        SkillSystem_start_select(sys);
    }
    if (sys->selecting) {
        ALLEGRO_KEYBOARD_STATE key_state;
        al_get_keyboard_state(&key_state);
        for (int i = 0; i < 3; i++) {
            if (al_key_down(&key_state, ALLEGRO_KEY_1 + i)) {
                sys->selected_skill = sys->options[i];
                sys->selecting = false;
                SkillSystem_apply(sys, player);
                printf("Selected skill: %s\n", skill_names[sys->selected_skill]);
                sys->last_select_time = now; // 技能選完後重設計時
            }
        }
        // Mouse click selection
        ALLEGRO_MOUSE_STATE mouse;
        al_get_mouse_state(&mouse);
        if (mouse.buttons & 1) {
            for (int i = 0; i < 3; i++) {
                int x = 150 + i * 250;
                int y = 200;
                int w = 120, h = 120;
                if (mouse.x >= x && mouse.x <= x + w && mouse.y >= y && mouse.y <= y + h) {
                    sys->selected_skill = sys->options[i];
                    sys->selecting = false;
                    SkillSystem_apply(sys, player);
                    printf("Selected skill: %s\n", skill_names[sys->selected_skill]);
                    sys->last_select_time = now; // 技能選完後重設計時
                }
            }
        }
    }
}

void SkillSystem_draw_menu(SkillSystem* sys) {
    if (!sys->selecting) return;
    static ALLEGRO_FONT* font = NULL;
    if (!font) {
        font = al_load_ttf_font("assets/font/pirulen.ttf", 24, 0);
        if (!font) {
            fprintf(stderr, "Failed to load font!\n");
            return;
        }
    }
    al_draw_filled_rectangle(0, 0, 900, 800, al_map_rgba(0,0,0,120));
    for (int i = 0; i < 3; i++) {
        char img_path[64];
        sprintf(img_path, "assets/image/skill_%d.png", sys->options[i]);
        ALLEGRO_BITMAP* bmp = al_load_bitmap(img_path);
        int x = 165 + i * 225;
        int y = 200;
        int center_x = x + 60;
        int center_y = y + 60;
        // 先畫底圖（正方形，白色半透明）
        al_draw_filled_rectangle(center_x - 60, center_y - 60, center_x + 60, center_y + 60, al_map_rgba(255,255,255,180));
        if (bmp) {
            int img_w = al_get_bitmap_width(bmp);
            int img_h = al_get_bitmap_height(bmp);
            al_draw_bitmap(bmp, center_x - img_w/2, center_y - img_h/2, 0);
            al_destroy_bitmap(bmp);
        } else {
            al_draw_filled_rectangle(x, y, x+120, y+120, al_map_rgb(80,80,80));
        }
        al_draw_textf(font, al_map_rgb(255,255,0), center_x, y + 170, ALLEGRO_ALIGN_CENTER, "%s", skill_names[sys->options[i]]);
        al_draw_textf(font, al_map_rgb(255,255,255), center_x, y + 210, ALLEGRO_ALIGN_CENTER, "Press %d", i+1);
    }
    al_draw_text(font, al_map_rgb(255,255,0), 450, 120, ALLEGRO_ALIGN_CENTER, "Choose a skill!");
}

void SkillSystem_apply(SkillSystem* sys, Character* player) {
    switch (sys->selected_skill) {
        case SKILL_HEAL_10P:
            player->hp += (int)(player->max_hp * 0.3);
            if (player->hp > player->max_hp) player->hp = player->max_hp;
            break;
        case SKILL_SHIELD_10P:
            player->shield_active = true;
            break;
        case SKILL_SPEED_10P:
            player->speed *= 1.3;
            break;
        case SKILL_MULTI_SHOT:
            player->multi_shot = true;
            player->multishot_level += 1; // <--- stackable
            break;
        default:
            break;
    }
}