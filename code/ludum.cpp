#include <base.cpp>

#include "ludum_mode_splash.cpp"
#include "ludum_mode_play.cpp"

function void LudumUpdateRender(Game_Context *context, Input *input, Renderer_Buffer *renderer_buffer) {
    Game_State *state = context->state;
    if (!state) {
        Memory_Allocator *system_alloc = Platform->GetMemoryAllocator();

        state = AllocInline(system_alloc, Gigabytes(1), Game_State, perm_arena);
        Initialise(&state->mode_arena, system_alloc, Gigabytes(1));

        Initialise(&state->audio_state, &state->perm_arena, V2(0.2f, 0.2f));
        Initialise(&state->assets, &state->perm_arena, context->texture_queue);

        context->state = state;
    }

    if (state->mode == GameMode_None) {
        // TODO @Anyone: Change before release
        //ModeSplash(state, input);
        ModePlay(state);
    }

    switch (state->mode) {
        case GameMode_Splash: {
            UpdateRenderModeSplash(state, input, renderer_buffer);
        }
        break;
        case GameMode_Play: {
            UpdateRenderModePlay(state, input, renderer_buffer);
        }
        break;
    }
}
