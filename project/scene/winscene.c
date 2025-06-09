#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "winscene.h"
#include <stdbool.h>
/*
   [Menu function]
*/
Scene* New_Winscene(int label)
{
    Winscene* pDerivedObj = (Winscene*)malloc(sizeof(Winscene));
    Scene* pObj = New_Scene(label);
    // setting derived object member
    //pDerivedObj->background = al_load_bitmap("assets/image/gamestartscene1.png");
    //pDerivedObj->font = al_load_ttf_font("assets/font/pirulen.ttf", 12, 0);
    // Load sound
    pDerivedObj->win_img = al_load_bitmap("assets/image/win.png");
    pDerivedObj->song = al_load_sample("assets/sound/menu.mp3");
    al_reserve_samples(20);
    pDerivedObj->sample_instance = al_create_sample_instance(pDerivedObj->song);
    // Loop the song until the display closes
    al_set_sample_instance_playmode(pDerivedObj->sample_instance, ALLEGRO_PLAYMODE_LOOP);
    al_restore_default_mixer();
    al_attach_sample_instance_to_mixer(pDerivedObj->sample_instance, al_get_default_mixer());
    // set the volume of instance
    al_set_sample_instance_gain(pDerivedObj->sample_instance, 0.1);
    pObj->pDerivedObj = pDerivedObj;
    // setting derived object function
    pObj->Update = Winscene_update;
    pObj->Draw = Winscene_draw;
    pObj->Destroy = Winscene_destroy;
    return pObj;
}

void Winscene_update(Scene* self)
{
    
    if (key_state[ALLEGRO_KEY_ENTER]&&flag==0)
    {
        self->scene_end = true;
        window = 2;
        printf("before flag=%d\n", flag);
        flag = 1;
        printf("flag=%d\n", flag);
    }
    else if (key_state[ALLEGRO_KEY_ENTER] && flag == 1)
    {
        self->scene_end = true;
        window = 3;
        printf("before flag=%d\n", flag);
        flag = 2;
        printf("flag=%d\n", flag);
    }
    else if (key_state[ALLEGRO_KEY_ENTER] && flag == 2)
    {
        self->scene_end = true;
        window = -1;
        printf("before flag=%d\n", flag);
        flag = 3;
        printf("flag=%d\n", flag);
    }
    return;
}
void Winscene_draw(Scene* self)
{
    Winscene* Obj = ((Winscene*)(self->pDerivedObj));
    if (Obj->win_img) {
        al_draw_bitmap(
            Obj->win_img,
            (WIDTH - al_get_bitmap_width(Obj->win_img)) / 2,
            (HEIGHT - al_get_bitmap_height(Obj->win_img)) / 2,
            0
        );
    }
    //al_draw_text(Obj->font, al_map_rgb(255, 255, 255), Obj->title_x, Obj->title_y, ALLEGRO_ALIGN_CENTRE, "Press 'Enter' to start");
    //al_draw_rectangle(Obj->title_x - 150, Obj->title_y - 30, Obj->title_x + 150, Obj->title_y + 30, al_map_rgb(255, 255, 255), 0);
    //al_play_sample_instance(Obj->sample_instance);
}
void Winscene_destroy(Scene* self)
{
    Winscene* Obj = ((Winscene*)(self->pDerivedObj));
    if (Obj->win_img)
        al_destroy_bitmap(Obj->win_img);
    //al_destroy_font(Obj->font);
    al_destroy_sample(Obj->song);
    //al_destroy_sample_instance(Obj->sample_instance);
    free(Obj);
    free(self);
    printf("winscene has destroyed.\n");
}
