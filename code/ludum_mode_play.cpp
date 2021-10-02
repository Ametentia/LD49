
function void ModePlay(Game_State *state, Random random) {
    Reset(&state->mode_arena);

    Mode_Play *play = AllocType(&state->mode_arena, Mode_Play);

    play->arena = &state->mode_arena;

    play->random = random;

    // Setup player
    //
    Player *player = &play->player;

    player->flags = 0;

    player->x_scale = 1;

    player->p   = V2(0.6, 0.6);
    player->dp  = V2(0, 0);

    player->birds[0].image  = GetImageByName(&state->assets, "bird_green");
    player->birds[0].cp     = 50;
    player->birds[0].vp     = 25;
    player->birds[0].offset = V2(-0.05, 0);

    player->birds[1].image  = GetImageByName(&state->assets, "bird_blue");
    player->birds[1].cp     = 35;
    player->birds[1].vp     = 12;
    player->birds[1].offset = V2(0, -0.18);

    player->birds[2].image = GetImageByName(&state->assets, "bird_red");
    player->birds[2].cp    = 40;
    player->birds[2].vp    = 30;
    player->birds[2].offset = V2(0.08, 0);

    player->dim = V2(0.3, 0.3);

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

    u32 count = Min(play->count, ArraySize(play->last_p));
    for (u32 it = 0; it < count; ++it) {
        DrawQuad(batch, { 0 }, play->last_p[it], V2(0.05, 0.05), 0, V4(0, 1, 0, 1));
    }

    Image_Handle image = GetImageByName(&state->assets, "main_standing_01");
    DrawQuad(batch, image, player->p, V2(player->x_scale, 1) * player->dim, 0, V4(1, 1, 1, 1));

#if 0
    else if (Dot(player_bird_dir, V2(1, 0)) < 0) {
        x_scale = -1;
    }
#endif

    for (u32 it = 0; it < ArraySize(player->birds); ++it) {
        Bird_Follower *bird = &player->birds[it];

        v2 player_bird_dir = (player->p + (V2(player->x_scale, 1) * bird->offset)) - bird->p;

        ddp = bird->cp * (player_bird_dir) - bird->vp * bird->dp;
        bird->p  += (0.5f * ddp * dt * dt) + (bird->dp * dt);
        bird->dp += (ddp * dt);

        bird->dir_timer -= input->delta_time;

        f32 x_scale = bird->x_scale;
        if (Length(ddp) < 0.05f) {
            x_scale = player->x_scale;
        }
        else if (bird->dir_timer <= 0) {
            bird->x_scale   = (RandomU32(&play->random, 0, 10) > 5) ? -1 : 1;
            bird->dir_timer = RandomF32(&play->random, 1, 3);
        }

        DrawQuad(batch, bird->image, bird->p, V2(0.3 * bird->x_scale, 0.3) * player->dim);
    }
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

        player->x_scale = -1;
    }

    // Move right
    //
    if (IsPressed(input->keys[Key_D])) {
        if (on_ground) { ddp.x = PLAYER_MOVE_SPEED; }
        else { ddp.x = PLAYER_AIR_STRAFE_SPEED; }

        player->x_scale = 1;
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
    if (player->p.y >= 2) {
        player->p.y = 2;
        player->flags |= Player_OnGround;
    }
}
