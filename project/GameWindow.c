#include "GameWindow.h"
#include "GAME_ASSERT.h"
#include "global.h"
#include "shapes/Shape.h"
// include allegro
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
// include scene and following component
#include "scene/sceneManager.h"
#include <stdbool.h>

Game *New_Game()
{
    Game *game = (Game *)malloc(sizeof(Game));
    game->execute = execute;
    game->game_init = game_init;
    game->game_update = game_update;
    game->game_draw = game_draw;
    game->game_destroy = game_destroy;
    game->title = "Final Project Team31";
    game->display = NULL;
    game->game_init(game);
    return game;
}
void execute(Game *self)
{
    // main game loop
    bool run = true;
    bool redraw = false;

    printf("Starting game loop...\n");

    while (run)
    {
        // process all events here
        al_wait_for_event(event_queue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            redraw = true;
            run &= self->game_update(self);
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            run = false;
            break;

        case ALLEGRO_EVENT_KEY_DOWN:
            printf("Key pressed: %d\n", event.keyboard.keycode);
            key_state[event.keyboard.keycode] = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                run = false;
            }
            break;

        case ALLEGRO_EVENT_KEY_UP:
            key_state[event.keyboard.keycode] = false;
            break;

        case ALLEGRO_EVENT_MOUSE_AXES:
            mouse.x = event.mouse.x;
            mouse.y = event.mouse.y;
            break;

        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            mouse_state[event.mouse.button] = true;
            break;

        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            mouse_state[event.mouse.button] = false;
            break;
        }

        // Redraw only when timer event occurred and no more events in queue
        if (redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            self->game_draw(self);
            al_flip_display();
        }
    }
    printf("Game loop ended.\n");
}
void game_init(Game *self)
{
    printf("Game Initializing...\n");
    if (!al_init()) {
        fprintf(stderr, "Failed to initialize Allegro.\n");
        exit(-1);
    }
    // initialize allegro addons
    bool addon_init = true;
    addon_init &= al_init_primitives_addon();
    addon_init &= al_init_font_addon();   // initialize the font addon
    addon_init &= al_init_ttf_addon();    // initialize the ttf (True Type Font) addon
    addon_init &= al_init_image_addon();  // initialize the image addon
    addon_init &= al_init_acodec_addon(); // initialize acodec addon
    addon_init &= al_install_keyboard();  // install keyboard event
    addon_init &= al_install_mouse();     // install mouse event
    addon_init &= al_install_audio();     // install audio event
    if (!addon_init) {
        fprintf(stderr, "Failed to initialize one or more Allegro addons.\n");
        exit(-1);
    }

    // Initialize audio mixer
    if (!al_reserve_samples(16)) {
        fprintf(stderr, "Failed to reserve audio samples.\n");
        exit(-1);
    }

    // Create event queue first
    event_queue = al_create_event_queue();
    if (!event_queue) {
        fprintf(stderr, "Failed to create event queue.\n");
        exit(-1);
    }

    // Create timer before display
    fps = al_create_timer(1.0 / FPS);
    if (!fps) {
        fprintf(stderr, "Failed to create timer.\n");
        exit(-1);
    }
    al_register_event_source(event_queue, al_get_timer_event_source(fps));

    // Create and initialize display
    self->display = al_create_display(WIDTH, HEIGHT);
    if (!self->display) {
        fprintf(stderr, "Failed to create display.\n");
        exit(-1);
    }
    
    // Initialize display settings
    al_set_window_position(self->display, 0, 0);
    al_set_window_title(self->display, self->title);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();

    // Register remaining event sources
    al_register_event_source(event_queue, al_get_display_event_source(self->display));
    
    // Make sure keyboard is installed and register its events
    if (!al_is_keyboard_installed()) {
        if (!al_install_keyboard()) {
            fprintf(stderr, "Failed to install keyboard!\n");
            exit(-1);
        }
    }
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    printf("Keyboard events registered\n");

    // Register mouse events
    if (!al_is_mouse_installed()) {
        if (!al_install_mouse()) {
            fprintf(stderr, "Failed to install mouse!\n");
            exit(-1);
        }
    }
    al_register_event_source(event_queue, al_get_mouse_event_source());
    printf("Mouse events registered\n");

    // Initialize all key states to false
    for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
        key_state[i] = false;
    }

    // Start timer
    al_start_timer(fps);

    // Try to load the icon, but don't crash if it fails
    ALLEGRO_BITMAP *icon = al_load_bitmap("assets/image/icon.jpg");
    if (icon) {
        al_set_display_icon(self->display, icon);
        al_destroy_bitmap(icon);  // Clean up the icon bitmap after setting it
    }

    printf("Game initialization completed.\n");
    
    // Create first scene last, after all systems are ready
    create_scene(Menu_L);
}
bool game_update(Game *self)
{
    scene->Update(scene);
    if (scene->scene_end)
    {
        printf("Scene ended. Window value: %d\n", window);
        scene->Destroy(scene);
        switch (window)
        {
        case 0:
            printf("Creating Menu Scene\n");
            create_scene(Menu_L);
            break;
        case 1:
            printf("Creating Game Scene\n");
            create_scene(GameScene_L);
            break;
        case 2:
            printf("Creating Game Scene 2\n");
            create_scene(GameScene2_L);
            break;
        case 3:
            printf("Creating Game Scene 3\n");
            create_scene(GameScene3_L);
            break;
        case 4:
            create_scene(WinScene_L);
            break;
        case -1:
            printf("Exiting game\n");
            return false;
        default:
            printf("Unknown window value: %d\n", window);
            break;
        }
    }
    return true;
}
void game_draw(Game *self)
{
    // Flush the screen first.
    al_clear_to_color(al_map_rgb(100, 100, 100));
    scene->Draw(scene);
    al_flip_display();
}
void game_destroy(Game *self)
{
    // Make sure you destroy all things
    al_destroy_event_queue(event_queue);
    al_destroy_display(self->display);
    scene->Destroy(scene);
    free(self);
}
