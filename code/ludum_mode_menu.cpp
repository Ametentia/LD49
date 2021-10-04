function void ModeMenu(Game_State *state, Random *random) {
    state->mode = GameMode_Menu;
    Mode_Menu *menu = AllocType(&state->mode_arena, Mode_Menu);
    menu->random = *random;
    state->menu = menu;
    Image_Handle menu_sheet = GetImageByName(&state->assets, "main_menu");
    Initialise(&menu->animation, menu_sheet, 1, 3, 1.0/6.0);
}

function void UpdateRenderModeMenu(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer) {
    f32 dt = input->delta_time;
    Draw_Batch _batch = {};
    Draw_Batch *batch = &_batch;
    Initialise(batch, &state->assets, renderer_buffer);
    Mode_Menu *menu = state->menu;
    SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(0, 0, 15));
    UpdateAnimation(&menu->animation, dt);
    DrawAnimation(batch, &menu->animation, V2(0,0), V2(10,10), 0, V4(1,1,1,1));
    for(u32 i = 0; i < Key_Count; i++) {
        if(JustPressed(input->keys[(Key_Code)i])) {
            ModePlay(state, menu->random);
        }
    }
}
