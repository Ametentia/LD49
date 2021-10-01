#include <base.cpp>

function void LudumUpdateRender(Game_Context *context, Input *input, Renderer_Buffer *renderer_buffer) {
    Game_State *state = context->state;
    if (!state) {
        Memory_Allocator *system_alloc = Platform->GetMemoryAllocator();

        state = AllocInline(system_alloc, Gigabytes(1), Game_State, perm);
        Initialise(&state->mode, system_alloc, Gigabytes(1));

        Initialise(&state->audio_state, &state->perm);
        Initialise(&state->assets, &state->perm, context->texture_queue);

        context->state = state;
    }

    Draw_Batch _batch = {};
    Draw_Batch *batch = &_batch;

    Initialise(batch, &state->assets, renderer_buffer);

    SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(0, 0, 8));

    if (IsPressed(input->keys[Key_W])) {
        DrawClear(batch, V4(1, 0, 1, 1));
    }
    else {
        DrawClear(batch, V4(0, 1, 1, 1));
    }
}
