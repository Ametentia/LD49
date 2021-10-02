
function void ModePlay(Game_State *state) {
    Reset(&state->mode_arena);

    Mode_Play *play = AllocType(&state->mode_arena, Mode_Play);

    play->arena = &state->mode_arena;

    // Setup player
    //
    Player *player = &play->player;

    player->flags = 0;

    player->p   = V2(0, 0);
    player->dp  = V2(0, 0);

    player->dim = V2(0.25f, 0.5f);

    state->mode = GameMode_Play;
    state->play = play;
}

function void UpdateRenderModePlay(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer) {
    Mode_Play *play = state->play;

    Draw_Batch _batch = {};
    Draw_Batch *batch = &_batch;

    Initialise(batch, &state->assets, renderer_buffer);

    Player *player = &play->player;
    UpdatePlayer(player, input);

    // @Debug: Showing player movement over time
    //
    play->last_p[play->next_last_p] = player->p;
    play->count += 1;
    play->next_last_p += 1;
    play->next_last_p %= ArraySize(play->last_p);

    // Camera movement
    //
    f32 dt = input->delta_time;

    v2 ddp = CAMERA_STIFFNESS * (player->p - play->camera_p) - CAMERA_DAMPING * play->camera_dp;
    play->camera_p  += (0.5f * ddp * dt * dt) + (play->camera_dp * dt);
    play->camera_dp += (ddp * dt);

    SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(play->camera_p, 8));

    DrawClear(batch, V4(0.01f, 0.01f, 0.01f, 1.0f));

    for (u32 it = 0; it < 20; ++it) {
        DrawQuad(batch, { 0 }, V2(0, it), V2(0.5, 0.5));
    }

    u32 count = Min(play->count, ArraySize(play->last_p));
    for (u32 it = 0; it < count; ++it) {
        DrawQuad(batch, { 0 }, play->last_p[it], V2(0.05, 0.05), 0, V4(0, 1, 0, 1));
    }

    DrawQuad(batch, { 0 }, player->p, player->dim, 0, V4(0, 1, 1, 1));
}

function void UpdatePlayer(Player *player, Input *input) {
    f32 dt = input->delta_time;

    b32 on_ground = (player->flags & Player_OnGround);

    f32 gravity = (2 * PLAYER_MAX_JUMP_HEIGHT) / (PLAYER_JUMP_APEX_TIME * PLAYER_JUMP_APEX_TIME);
    v2 ddp      = V2(0, gravity);

    // Attempt to jump. We will buffer this for an amount of time so if the player presses jump
    // slightly before hitting the ground it will still count
    //
    if (JustPressed(input->keys[Key_K])) {
        player->last_jump_time = input->time;
    }

    // Move left
    //
    if (IsPressed(input->keys[Key_A])) {
        if (on_ground) { ddp.x = -PLAYER_MOVE_SPEED; }
        else { ddp.x = -PLAYER_AIR_STRAFE_SPEED; }
    }

    // Move right
    //
    if (IsPressed(input->keys[Key_D])) {
        if (on_ground) { ddp.x = PLAYER_MOVE_SPEED; }
        else { ddp.x = PLAYER_AIR_STRAFE_SPEED; }
    }

    // If neither left or right were pressed apply damping to the player
    //
    if (IsZero(ddp.x)) {
        player->dp.x *= (1.0f / (1 + (PLAYER_DAMPING * dt)));
    }

    if (on_ground) {
        if ((input->time - player->last_jump_time) <= PLAYER_JUMP_BUFFER_TIME) {
            player->dp.y   = -Sqrt(2 * gravity * PLAYER_MAX_JUMP_HEIGHT);
            player->flags &= ~Player_OnGround;
        }
    }

    if (!IsPressed(input->keys[Key_K]) && (player->dp.y < 0)) {
        f32 initial_dp_sq = (2 * gravity * PLAYER_MAX_JUMP_HEIGHT);
        f32 limit_dp_sq   = (2 * gravity * (PLAYER_MAX_JUMP_HEIGHT - PLAYER_MIN_JUMP_HEIGHT));

        f32 term_dp = -Sqrt(initial_dp_sq - limit_dp_sq);
        if (player->dp.y < term_dp) {
            player->dp.y = term_dp;
        }
    }

    player->p  += (player->dp * dt);
    player->dp += (ddp * dt);

    // Limit x speed
    //
    if (Abs(player->dp.x) > PLAYER_MAX_SPEED_X) {
        player->dp.x *= (PLAYER_MAX_SPEED_X / Abs(player->dp.x));
    }

    // @Temp: Hardcoded floor line for testing
    //
    if (player->p.y >= 5) {
        player->p.y = 5;
        player->flags |= Player_OnGround;
    }
}
