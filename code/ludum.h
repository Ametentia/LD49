#if !defined(LUDUM_H_)
#define LUDUM_H_

#include <base.h>

struct Game_State;

#include "ludum_mode_menu.h"
#include "ludum_mode_splash.h"
#include "ludum_mode_play.h"
#include "ludum_mode_minigame.h"

enum Game_Mode {
    GameMode_None = 0,
    GameMode_Splash,
    GameMode_Play,
    GameMode_Menu,
    GameMode_MiniGame
};

struct Game_State {
    Memory_Arena perm_arena;
    Memory_Arena mode_arena;

    Audio_State audio_state;
    Asset_Manager assets;

    Game_Mode mode;
    union {
        Mode_Splash   *splash;
        Mode_Play     *play;
        Mode_Menu     *menu;
    };
};

struct Game_Context {
    Texture_Transfer_Queue *texture_queue;

    Game_State *state;
};

#endif  // LUDUM_H_
