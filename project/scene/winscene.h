#ifndef WINSCENE_H_INCLUDED
#define WINSCENE_H_INCLUDED
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include "scene.h"
extern int flag;
/*
   [Menu object]
*/
typedef struct _Winscene
{
    ALLEGRO_BITMAP* background;
    ALLEGRO_FONT* font;
    ALLEGRO_SAMPLE* song;
    ALLEGRO_SAMPLE_INSTANCE* sample_instance;
    ALLEGRO_BITMAP* win_img;
    int flag;
} Winscene;
Scene* New_Winscene(int label);
void Winscene_update(Scene* self);
void Winscene_draw(Scene* self);
void Winscene_destroy(Scene* self);

#endif
