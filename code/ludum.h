#if !defined(LUDUM_H_)
#define LUDUM_H_

#include <base.h>
#include "splash.h"

struct Game_State {
    Memory_Arena perm;
    Memory_Arena mode;

    Audio_State audio_state;
    Asset_Manager assets;

    Mode_Splash splash;
};

struct Game_Context {
    Texture_Transfer_Queue *texture_queue;

    Game_State *state;
};

#endif  // LUDUM_H_
