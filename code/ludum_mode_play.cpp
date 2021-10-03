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

    player->p = V2(WORLD_TILE_SIZE*4, (WORLD_Y_SIZE - 3)*WORLD_TILE_SIZE);
    play->camera_p = V2(WORLD_TILE_SIZE*4, (WORLD_Y_SIZE - 3)*WORLD_TILE_SIZE);
    play->camera_dp = V2(0, 0);
    player->dp  = V2(0, 0);

    player->current_animation = Player_Idle;

    Image_Handle idle_handle = GetImageByName(&state->assets, "idle");
    Initialise(&(player->animations[Player_Idle]), idle_handle, 1, 6, 1.0/24.0);

    Image_Handle run_handle = GetImageByName(&state->assets, "running");
    Initialise(&(player->animations[Player_Run]), run_handle, 1, 9, 1.0/24.0);

    player->birds[0].image  = GetImageByName(&state->assets, "bird_green");
    player->birds[0].cp     = 50;
    player->birds[0].vp     = 25;
    player->birds[0].offset = V2(-0.05, 0);

    player->birds[1].image  = GetImageByName(&state->assets, "bird_blue");
    player->birds[1].cp     = 35;
    player->birds[1].vp     = 12;
    player->birds[1].offset = V2(0, -0.18);

    player->birds[2].image  = GetImageByName(&state->assets, "bird_red");
    player->birds[2].cp     = 40;
    player->birds[2].vp     = 30;
    player->birds[2].offset = V2(0.08, 0);

    player->dim           = V2(0.15, 0.3);

    player->visual_dim    = V2(0.5, 0.5);
    player->visual_offset = V2(0, -0.1);

    play->debug_camera_p  = V3(0, 0, 5);

    play->tiles = AllocArray(play->arena, Tile, (WORLD_X_SIZE * WORLD_Y_SIZE));
    SpawnPopulation(play->tiles, &random);
    SimGeneration(play->tiles, 15);

    state->mode = GameMode_Play;
    state->play = play;
}

function void UpdateRenderModePlay(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer) {
    Mode_Play *play = state->play;

    Draw_Batch _batch = {};
    Draw_Batch *batch = &_batch;

    Initialise(batch, &state->assets, renderer_buffer);

    Player *player = &play->player;
    Tile *tiles = play->tiles;
    // @Debug: regen world
    //
    if(JustPressed(input->keys[Key_F3])) {
        SpawnPopulation(play->tiles, &(play->random));
        SimGeneration(tiles, 15);
        player->p = V2(WORLD_TILE_SIZE*4, (WORLD_Y_SIZE - 3)*WORLD_TILE_SIZE);
        play->camera_p = V2(WORLD_TILE_SIZE*4, (WORLD_Y_SIZE - 3)*WORLD_TILE_SIZE);
        play->camera_dp = V2(0, 0);
    }

    if (JustPressed(input->keys[Key_F2])) {
        play->debug_camera_enabled = !play->debug_camera_enabled;
    }

    if (IsPressed(input->keys[Key_Alt])) {
        if (IsPressed(input->mouse_buttons[Mouse_Left])) {
            f32 speed_scale = (play->debug_camera_p.z / 5.0f);
            play->debug_camera_p.xy += speed_scale * V2(-input->mouse_delta.x, input->mouse_delta.y);
        }
    }

    play->debug_camera_p.z -= input->mouse_delta.z;
    play->debug_camera_p.z = Clamp(play->debug_camera_p.z, 4, 150);

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

    if (play->debug_camera_enabled) {
        SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), play->debug_camera_p);
    }
    else if (IsPressed(input->keys[Key_P])) {
        SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(play->camera_p, 150));
    }
    else {
        SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(play->camera_p, 10));
    }

    DrawClear(batch, V4(0.01f, 0.01f, 0.01f, 1.0f));

    UpdatePlayer(play, player, input, state);

    // @Debug: Showing player movement over time
    //
    play->last_p[play->next_last_p] = player->p;
    play->count += 1;
    play->next_last_p += 1;
    play->next_last_p %= ArraySize(play->last_p);

    Image_Handle ground_image = GetImageByName(&state->assets, "ground_01");
    v2 tile_dim = V2(WORLD_TILE_SIZE, WORLD_TILE_SIZE);

    for (u32 y = 0; y < WORLD_Y_SIZE; ++y) {
        for (u32 x = 0; x < WORLD_X_SIZE; ++x) {
            Tile *tile = &play->tiles[(y * WORLD_Y_SIZE) + x];
            if (tile->type == Tile_Air) { continue; }

            v2 world_p = V2(tile->grid_p) * tile_dim;
            DrawQuad(batch, ground_image, world_p, tile_dim);
        }
    }

    u32 count = Min(play->count, ArraySize(play->last_p));
    for (u32 it = 0; it < count; ++it) {
        DrawQuad(batch, { 0 }, play->last_p[it], V2(0.05, 0.05), 0, V4(0, 1, 0, 1));
    }

    DrawAnimation(batch, &player->animations[player->current_animation], player->p + player->visual_offset,
            V2(player->x_scale, 1) * player->visual_dim);

    DrawQuadOutline(batch, player->p, player->dim, 0, V4(1, 0, 0, 1), 0.01);
    if(player->flags & Player_Drilling) {
        Image_Handle drill = GetImageByName(&state->assets, "drill");
        v2 drill_pos = player->p + V2(0.08 * player->x_scale, 0.04);
        DrawQuad(batch, drill, drill_pos, V2(0.3 *player->x_scale, 0.1), 0, V4(1, 1, 1, 1));
    }

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

        DrawQuad(batch, bird->image, bird->p, V2(0.3 * bird->x_scale, 0.3)*0.4);
    }
}

function b32 Overlaps(rect2 a, rect2 b) {
    b32 result =
        (a.max.x >= b.min.x) && (a.min.x <= b.max.x) &&
        (a.max.y >= b.min.y) && (a.min.y <= b.max.y);

    return result;
}

function f32 Sign(f32 x) {
    f32 result = (x < 0) ? -1 : 1;
    return result;
}

function b32 IsValidTile(v2s tile_p) {
    b32 result =
        (tile_p.x >= 0) && (tile_p.x < WORLD_X_SIZE) &&
        (tile_p.y >= 0) && (tile_p.y < WORLD_Y_SIZE);

    return result;
}

function u32 GetCloseTiles(v2 p, Tile *tiles, Tile **out) {
    u32 result = 0;

    v2s p_grid;
    p_grid.x = cast(s32) ((p.x / WORLD_TILE_SIZE) + 0.5f);
    p_grid.y = cast(s32) ((p.y / WORLD_TILE_SIZE) + 0.5f);

    for (s32 y = -1; y <= 1; ++y) {
        for (s32 x = -1; x <= 1; ++x) {
            v2s tile_p = V2S(p_grid.x + x, p_grid.y + y);

            if (IsValidTile(tile_p)) {
                out[result] = &tiles[(tile_p.y * WORLD_Y_SIZE) + tile_p.x];
                result += 1;
            }
        }
    }

    return result;
}

function void UpdatePlayer(Mode_Play *play, Player *player, Input *input, Game_State *state) {
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
    if ((input->time - player->last_jump_time) <= PLAYER_JUMP_BUFFER_TIME) {
        if (on_ground || (input->time - player->last_on_ground_time) <= PLAYER_COYOTE_TIME) {
            player->dp.y   = -Sqrt(2 * gravity * PLAYER_MAX_JUMP_HEIGHT);
            player->flags &= ~Player_OnGround;
            player->last_jump_time = 0;
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
    //f32 max_speed = (player->flags & Player_Drilling) ? PLAYER_MAX_SPEED_X_DRILLING : PLAYER_MAX_SPEED_X;
    f32 max_speed = PLAYER_MAX_SPEED_X;
    if (Abs(player->dp.x) > max_speed) {
        player->dp.x *= (max_speed / Abs(player->dp.x));
    }

    // Drill
    //
    if(IsPressed(input->keys[Key_L]) && on_ground) {
        if(!(player->flags & Player_Drilling)) {
            // TODO Drill noise
        }
        player->flags |= Player_Drilling;
    } else if(player->flags & Player_Drilling){
        player->flags &= ~Player_Drilling;
        // TODO stop Drill noise
    }

    // Calculate the player collision box
    //
    rect2 player_r;
    player_r.min = player->p - (0.5f * player->dim);
    player_r.max = player->p + (0.5f * player->dim);

    // Calculate the drill collision box
    //
    rect2 drill_r;
    v2 drill_pos = player->p + V2(0.08 * player->x_scale, -0.15f);
    v2 drill_dim = V2(0.3, 0.4);
    drill_r.min = drill_pos - (0.5f * drill_dim);
    drill_r.max = drill_pos + (0.5f * drill_dim);

    Tile *collision_tiles[9] = {};
    u32 tile_count = GetCloseTiles(player->p, play->tiles, collision_tiles);
    player->flags &= ~Player_OnGround;

    v2 tile_dim = V2(WORLD_TILE_SIZE, WORLD_TILE_SIZE);
    u8 hits = 0;
    if(player->flags & Player_Drilling) {
        for (u32 it = 0; it < tile_count; ++it) {
            Tile *tile = collision_tiles[it];
            if (tile->type == Tile_Air) { continue; }

            v2 tile_p = V2(tile->grid_p) * tile_dim;

            rect2 tile_r;
            tile_r.min = tile_p - (0.5f * tile_dim);
            tile_r.max = tile_p + (0.5f * tile_dim);
            if (Overlaps(drill_r, tile_r)) {
                f32 x_over = Min(drill_r.max.x, tile_r.max.x) - Max(drill_r.min.x, tile_r.min.x);
                if(x_over > 0) {
                    tile->drill_time += dt;
                    if(tile->drill_time > TILE_DRILL_TIME) {
                        tile->type = Tile_Air;
                    } else {
                        hits++;
                        if(!player->drill_hit_playing) {
                            Sound_Handle drill = GetSoundByName(&state->assets, "drillbrrr");
                            player->drill_hit_sound = *PlaySound(&state->audio_state, drill, PlayingSound_Looped);
                            player->drill_hit_playing = 1;
                        }
                    }
                }
            } else {
                tile->drill_time = 0;
            }
        }
    }
    if(hits == 0 && player->drill_hit_playing) {
        StopSound(&state->audio_state, &player->drill_hit_sound);
        player->drill_hit_playing = 0;
    }
    for (u32 it = 0; it < tile_count; ++it) {
        Tile *tile = collision_tiles[it];
        if (tile->type == Tile_Air) { continue; }

        v2 tile_p = V2(tile->grid_p) * tile_dim;

        rect2 tile_r;
        tile_r.min = tile_p - (0.5f * tile_dim);
        tile_r.max = tile_p + (0.5f * tile_dim);

        if (Overlaps(player_r, tile_r)) {
            v2 overlap;

            overlap.x = Min(player_r.max.x, tile_r.max.x) - Max(player_r.min.x, tile_r.min.x);
            overlap.y = Min(player_r.max.y, tile_r.max.y) - Max(player_r.min.y, tile_r.min.y);

            v2 dir = (player->p - tile_p);

            if (overlap.x <= overlap.y) {
                if (overlap.x > 0) {
                    f32 sign = Sign(dir.x);
                    player->p.x += (overlap.x * sign);

                    player->dp.x = 0;
                }
            }
            else {
                f32 sign = Sign(dir.y);
                player->p.y += (overlap.y * sign);

                if (sign < 0) {
                    player->flags |= Player_OnGround;
                    player->dp.y = 0;

                    player->last_on_ground_time = input->time;
                }
                else {
                    player->dp.y = 0;
                    player->dp.y += (ddp.y * dt);
                }
            }
        }
    } 
}
