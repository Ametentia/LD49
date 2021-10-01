#include <base.cpp>
#include "splash.cpp"

function void LudumUpdateRender(Game_Context *context, Input *input, Renderer_Buffer *renderer_buffer) {
    Game_State *state = context->state;
    if (!state) {
        Memory_Allocator *system_alloc = Platform->GetMemoryAllocator();

        state = AllocInline(system_alloc, Gigabytes(1), Game_State, perm);
        Initialise(&state->mode, system_alloc, Gigabytes(1));

        Initialise(&state->audio_state, &state->perm, V2(0.2, 0.2));
        Initialise(&state->assets, &state->perm, context->texture_queue);

        context->state = state;
        ModeSplash(state, input);
    }
    UpdateRenderModeSplash(state, input, renderer_buffer);
}
