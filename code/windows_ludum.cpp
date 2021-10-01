#include "ludum.h"
#include "ludum.cpp"

// Windows specific stuff
//
#include <platform/windows.h>
#include <platform/windows.cpp>

int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR args, int show_cmd) {
    (void) instance;
    (void) prev_instance;
    (void) args;

    OutputDebugStringA("Hello, Windows!\n");

    // Initialise Windows platform subsystem
    //
    Windows_Parameters init_params = {};
    init_params.init_flags = PlatformInit_OpenWindow | PlatformInit_EnableAudio;

    init_params.show_cmd     = show_cmd;
    init_params.window_dim   = V2U(1280, 720);
    init_params.window_title = WrapConst("LD49");

    if (!WindowsInitialise(&init_params)) {
        return 1;
    }

    // Load and initialise the renderer
    //
    Renderer_Parameters renderer_params = {};
    renderer_params.command_buffer_size = Megabytes(1);
    renderer_params.max_immediate_quads = (1 << 18);

    renderer_params.max_texture_handles = 512;
    renderer_params.texture_queue_size  = Megabytes(256);

    Renderer_Context *renderer = WindowsLoadRenderer(&renderer_params);
    if (!IsValid(renderer)) {
        return 1;
    }

    // Setup input structure
    //
    Input _input = {};
    Input *input = &_input;

    // Game context
    //
    Game_Context _context = {};
    Game_Context *context = &_context;

    context->state         = 0;
    context->texture_queue = &renderer->texture_queue;

    b32 running = true;
    while (running) {
        WindowsHandleInput(input);
        if (input->requested_quit) { running = false; }

        v2u window_dim = WindowsGetWindowDim();

        rect2 render_region = {};
        render_region.min = V2(0, 0);
        render_region.max = V2(window_dim);

        Renderer_Buffer *renderer_buffer = renderer->BeginFrame(renderer, window_dim, render_region);

        LudumUpdateRender(context, input, renderer_buffer);

        renderer->SubmitFrame(renderer);

        Game_State *state = context->state;
        Audio_Buffer audio_buffer = {};
        if (WindowsGetAudioBuffer(&audio_buffer)) {
            MixPlayingSounds(&state->audio_state, &state->assets,  &audio_buffer);
            WindowsSubmitAudioBuffer(&audio_buffer);
        }
    }

    return 0;
}
