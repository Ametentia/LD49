#include "ludum_world_gen.cpp"
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
    player->time_off_ground = 0;
    player->current_animation = Player_Idle;

    Image_Handle idle_handle = GetImageByName(&state->assets, "idle");
    Initialise(&(player->animations[Player_Idle]), idle_handle, 1, 6, 1.0/24.0);

    Image_Handle run_handle = GetImageByName(&state->assets, "Walking_sheet");
    Initialise(&(player->animations[Player_Run]), run_handle, 1, 20, 1.0/64.0);

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

    player->dim = V2(0.5, 0.5);

    // Setup World
    Tile (*tiles)[WORLD_Y_SIZE] = play->tiles;
    SpawnPopulation(tiles, &random);
    SimGeneration(tiles, 5);
    for(int i = 0; i < WORLD_X_SIZE; i++){
        for(int j = 0; j < WORLD_Y_SIZE; j++){
            tiles[i][j].p = V2(i*WORLD_TILE_SIZE, j*WORLD_TILE_SIZE);
            tiles[i][j].dim = V2(WORLD_TILE_SIZE, WORLD_TILE_SIZE);
        }
    }

    state->mode = GameMode_Play;
    state->play = play;
}

function void UpdateRenderModePlay(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer) {
    Mode_Play *play = state->play;

    Draw_Batch _batch = {};
    Draw_Batch *batch = &_batch;

    Initialise(batch, &state->assets, renderer_buffer);

    Player *player = &play->player;
    UpdatePlayer(player, input, play, batch);
    Tile (*tiles)[WORLD_Y_SIZE] = play->tiles;
    if(JustPressed(input->keys[Key_L])) {
        SpawnPopulation(tiles, &(play->random));
        SimGeneration(tiles, 10);
    }

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

    SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(play->camera_p, 180));

    DrawClear(batch, V4(0.01f, 0.01f, 0.01f, 1.0f));

    // Draw World tiles
    Image_Handle tile_sprite = GetImageByName(&state->assets, "ground_01");
    for(int i = 0; i < WORLD_X_SIZE; i++){
        for(int j = 0; j < WORLD_Y_SIZE; j++){
            Tile t = tiles[i][j];
            if(t.alive) {
                DrawQuad(batch, tile_sprite, t.p, V2(WORLD_TILE_SIZE, WORLD_TILE_SIZE), 0, V4(1,1,1,1));
            }
            int x_right = t.p.x + t.dim.x/2.0 > player->p.x - player->dim.x/2.0;
            int x_left = t.p.x - t.dim.x/2.0 < player->p.x + player->dim.x/2.0;
            int y_up = t.p.y + t.dim.y/2.0 > player->p.y - player->dim.y/2.0;
            int y_down = t.p.y - t.dim.y/2.0 < player->p.y + player->dim.y/2.0;
            if(x_right && x_left && y_up && y_down && t.alive) {
                DrawQuad(batch, tile_sprite, t.p, V2(WORLD_TILE_SIZE, WORLD_TILE_SIZE), 0, V4(1,0,0,1));
            }
        }
    }

    u32 count = Min(play->count, ArraySize(play->last_p));
    for (u32 it = 0; it < count; ++it) {
        DrawQuad(batch, { 0 }, play->last_p[it], V2(0.05, 0.05), 0, V4(0, 1, 0, 1));
    }

    DrawAnimation(
        batch,
        &(player->animations[player->current_animation]),
        player->p,
        V2(player->x_scale, 1) * player->dim,
        0,
        V4(1,1,1,1)
    );

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

        //DrawQuad(batch, bird->image, bird->p, V2(0.3 * bird->x_scale, 0.3)*0.4);
    }
}

function void UpdatePlayer(Player *player, Input *input, Mode_Play *play, Draw_Batch *batch) {
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
    UpdateAnimation(&(player->animations[player->current_animation]), dt);

    // Move left
    //
    if (IsPressed(input->keys[Key_A])) {
        if (on_ground) { ddp.x = -PLAYER_MOVE_SPEED; }
        else { ddp.x = -PLAYER_AIR_STRAFE_SPEED; }

        player->current_animation = Player_Run;
        player->x_scale = -1;
    }
    // Move right
    //
    if (IsPressed(input->keys[Key_D])) {
        if (on_ground) { ddp.x = PLAYER_MOVE_SPEED; }
        else { ddp.x = PLAYER_AIR_STRAFE_SPEED; }

        player->x_scale = 1;
        player->current_animation = Player_Run;
    }

    // If neither left or right were pressed apply damping to the player
    //
    if (IsZero(ddp.x)) {
        player->dp.x *= (1.0f / (1 + (PLAYER_DAMPING * dt)));
        player->current_animation = Player_Idle;
    }

    if (player->time_off_ground < PLAYER_COYOTE_TIME || on_ground) {
        if ((input->time - player->last_jump_time) <= PLAYER_JUMP_BUFFER_TIME && on_ground) {
            player->dp.y   = -Sqrt(2 * gravity * PLAYER_MAX_JUMP_HEIGHT);
            player->flags &= ~Player_OnGround;
            player->time_off_ground = 0;
        }
    } 
    if(!on_ground){
        player->time_off_ground += dt;
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

    Tile (*tiles)[WORLD_Y_SIZE] = play->tiles;
    for(int i = 0; i < WORLD_X_SIZE; i++){
        for(int j = 0; j < WORLD_Y_SIZE; j++){
            Tile t = tiles[i][j];
            if(!t.alive) {
                continue;
            }
            int x_right = t.p.x + t.dim.x/2.0 > player->p.x - player->dim.x/2.0;
            int x_left = t.p.x - t.dim.x/2.0 < player->p.x + player->dim.x/2.0;
            int y_up = t.p.y + t.dim.y/2.0 > player->p.y - player->dim.y/2.0;
            int y_down = t.p.y - t.dim.y/2.0 < player->p.y + player->dim.y/2.0;
            if(x_right && x_left && y_up && y_down) {
                player->p.y = t.p.y - player->dim.y/2;
                player->flags |= Player_OnGround;
                player->dp.y = 0;
            } 
        }
    }
}
