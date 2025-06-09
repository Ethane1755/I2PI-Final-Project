#include "sceneManager.h"
#include "menu.h"
#include "gamescene.h"
#include "gamescene2.h"
#include "winscene.h"
#include "gamescene3.h"
Scene* scene = NULL;
void create_scene(SceneType type)
{
    switch (type)
    {
    case Menu_L:
        scene = New_Menu(Menu_L);
        break;
    case GameScene_L:
        scene = New_GameScene(GameScene_L);
        break;
    case GameScene2_L:
        scene = New_GameScene2(GameScene2_L);
        break;
    case GameScene3_L:
        scene = New_GameScene3(GameScene3_L);
        break;
    case WinScene_L:
        scene = New_Winscene(WinScene_L);
        break;
    default:
        // Keep current scene if type is invalid
        break;
    }
}
