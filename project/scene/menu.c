#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "menu.h"
#include <stdbool.h>
/*
   [Menu function]
*/
Scene *New_Menu(int label)
{
    Menu *pDerivedObj = (Menu *)malloc(sizeof(Menu));
    Scene *pObj = New_Scene(label);
    // setting derived object member
    //pDerivedObj->background = al_load_bitmap("assets/image/gamestartscene1.png");
    pDerivedObj->font = al_load_ttf_font("assets/font/pirulen.ttf", 12, 0);
    if (!pDerivedObj->font) {
        fprintf(stderr, "Failed to load font, trying default font...\n");
        // Try to load a system font as fallback
        pDerivedObj->font = al_create_builtin_font();
        if (!pDerivedObj->font) {
            fprintf(stderr, "Failed to load default font!\n");
            exit(-1);
        }
    }
    // Set menu position
    pDerivedObj->title_x = WIDTH / 2;
    pDerivedObj->title_y = HEIGHT / 2;

    // Load and setup sound
    pDerivedObj->song = al_load_sample("assets/sound/menu.mp3");
    if (!pDerivedObj->song) {
        fprintf(stderr, "Failed to load menu.mp3\n");
        // Don't exit, just continue without sound
        pDerivedObj->sample_instance = NULL;
    } else {
        pDerivedObj->sample_instance = al_create_sample_instance(pDerivedObj->song);
        if (pDerivedObj->sample_instance) {
            al_set_sample_instance_playmode(pDerivedObj->sample_instance, ALLEGRO_PLAYMODE_LOOP);
            al_attach_sample_instance_to_mixer(pDerivedObj->sample_instance, al_get_default_mixer());
            al_set_sample_instance_gain(pDerivedObj->sample_instance, 0.1);
        }
    }
    pObj->pDerivedObj = pDerivedObj;
    // setting derived object function
    pObj->Update = menu_update;
    pObj->Draw = menu_draw;
    pObj->Destroy = menu_destroy;
    return pObj;
}
void menu_update(Scene *self)
{
    static bool enter_pressed = false;

    // Check for Enter key press
    if (key_state[ALLEGRO_KEY_ENTER]) {
        if (!enter_pressed) {  // Only trigger once when key is first pressed
            printf("Enter key pressed! Transitioning to game scene...\n");
            self->scene_end = true;
            window = 1;
            printf("Set window to %d and scene_end to true\n", window);
            enter_pressed = true;
        }
    } else {
        enter_pressed = false;  // Reset when key is released
    }

    // Debug: print if any key is pressed
    for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
        if (key_state[i]) {
            printf("Key %d is pressed\n", i);
        }
    }
}
void menu_draw(Scene *self)
{
    Menu *Obj = ((Menu *)(self->pDerivedObj));
    if (Obj->font) {
        al_draw_text(Obj->font, al_map_rgb(255, 255, 255), Obj->title_x, Obj->title_y, ALLEGRO_ALIGN_CENTRE, "Press 'Enter' to start");
        al_draw_rectangle(Obj->title_x - 150, Obj->title_y - 30, Obj->title_x + 150, Obj->title_y + 30, al_map_rgb(255, 255, 255), 0);
    }
    if (Obj->sample_instance) {
        al_play_sample_instance(Obj->sample_instance);
    }
}

void menu_destroy(Scene *self)
{
    Menu *Obj = ((Menu *)(self->pDerivedObj));
    if (Obj->font) {
        al_destroy_font(Obj->font);
    }
    if (Obj->song) {
        al_destroy_sample(Obj->song);
    }
    if (Obj->sample_instance) {
        al_destroy_sample_instance(Obj->sample_instance);
    }
    free(Obj);
    free(self);
}
