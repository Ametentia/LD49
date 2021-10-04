#include "ludum_world_gen.cpp"

function void ModePlay(Game_State *state, Random random) {
    Reset(&state->mode_arena);

    Mode_Play *play = AllocType(&state->mode_arena, Mode_Play);

    play->arena  = &state->mode_arena;
    play->random = random;

    // Setup player
    //
    Player *player = &play->player;

    player->flags = 0;

    player->p      = V2(WORLD_TILE_SIZE * 4, (WORLD_Y_SIZE - 3) * WORLD_TILE_SIZE);
    player->dp     = V2(0, 0);
    player->dim    = V2(0.15, 0.3); // This is used for collision detection
    player->facing = 1;

    // These are used to draw the player
    //
    player->visual_dim    = V2(0.5, 0.5);
    player->visual_offset = V2(0,  -0.1);

    player->drill_p  = player->p;
    player->drill_dp = V2(0, 0);

    str8 animation_names[PlayerAnimation_Count] = {
        WrapConst("idle"),
        WrapConst("running")
    };

    u32 frame_counts[PlayerAnimation_Count] = { 6, 9 }; // Nice

    player->cur_anim   = PlayerAnimation_Idle;
    player->animations = AllocArray(play->arena, Sprite_Animation, PlayerAnimation_Count);

    for (u32 it = 0; it < PlayerAnimation_Count; ++it) {
        Image_Handle handle = GetImageByName(&state->assets, animation_names[it]);
        Initialise(&player->animations[it], handle, 1, frame_counts[it], 1.0f / 24.0f);
    }

    Image_Handle attack_sheet = GetImageByName(&state->assets, "attack");
    Initialise(&player->attack_anim, attack_sheet, 1, 6, 1.0f / 24.0f);

    Image_Handle drill_sheet = GetImageByName(&state->assets, "drilling");
    Initialise(&player->drill_anim, drill_sheet, 1, 3, 1.0f / 24.0f);

    // Setup bird followers
    //
    Bird_Follower *birds = player->birds;

    birds[0].image  = GetImageByName(&state->assets, "bird_green");
    birds[0].p      = player->p;
    birds[0].cp     = 50;
    birds[0].vp     = 25;
    birds[0].offset = V2(-0.05, 0);

    birds[1].image  = GetImageByName(&state->assets, "bird_blue");
    birds[1].p      = player->p;
    birds[1].cp     = 35;
    birds[1].vp     = 12;
    birds[1].offset = V2(0, -0.18);

    birds[2].image  = GetImageByName(&state->assets, "bird_red");
    birds[2].p      = player->p;
    birds[2].cp     = 40;
    birds[2].vp     = 30;
    birds[2].offset = V2(0.08, 0);

    // Camera parameters
    //
    play->camera_p     = player->p + V2(12 * WORLD_TILE_SIZE, -15 * WORLD_TILE_SIZE);
    play->shake_angle  = Pi32 / 64.0f;
    play->shake_offset = V2(0.10, 0.10);

    play->debug_camera_p = V3(0, 0, 5);

    play->tiles = AllocArray(play->arena, Tile, (WORLD_X_SIZE * WORLD_Y_SIZE));
    GenerateWorld(play, &random, &state->assets);

    play->max_particles = 1024;
    play->next_particle = 0;
    play->particle_cache = AllocArray(play->arena, Particle, play->max_particles);

    Sound_Handle music_handle = GetSoundByName(&state->assets, "bgm");
    play->music = PlaySound(&state->audio_state, music_handle, PlayingSound_Looped, V2(0.0, 0.0));

    state->mode = GameMode_Play;
    state->play = play;
}

function void UpdateRenderModePlay(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer) {
    Mode_Play *play = state->play;

    Draw_Batch _batch = {};
    Draw_Batch *batch = &_batch;

    Initialise(batch, &state->assets, renderer_buffer);

    Player *player = &play->player;

    // @Debug: Regenerate world
    //
    if (JustPressed(input->keys[Key_F3])) {
        play->enemy_count = 0;

        GenerateWorld(play, &play->random, &state->assets);

        player->p       = V2(WORLD_TILE_SIZE * 4, (WORLD_Y_SIZE - 3) * WORLD_TILE_SIZE);
        play->camera_p  = player->p + V2(12 * WORLD_TILE_SIZE, -15 * WORLD_TILE_SIZE);
        play->camera_dp = V2(0, 0);
    }

    if (JustPressed(input->keys[Key_F2])) {
        play->debug_camera_enabled = !play->debug_camera_enabled;
    }

    if (play->debug_camera_enabled) {
        if (IsPressed(input->keys[Key_Alt])) {
            if (IsPressed(input->mouse_buttons[Mouse_Left])) {
                f32 speed_scale = (play->debug_camera_p.z / 5.0f);
                play->debug_camera_p.xy += speed_scale * V2(-input->mouse_delta.x, input->mouse_delta.y);
            }
        }

        // @Debug: Update debug camera
        //
        play->debug_camera_p.z -= input->mouse_delta.z;
        play->debug_camera_p.z = Clamp(play->debug_camera_p.z, 4, 150);
    }

    // Camera movement
    //
    f32 dt = input->delta_time;

    play->shake_t -= dt;
    play->shake   -= dt;
    play->shake    = Max(play->shake, 0);

    if (play->shake_t <= 0) {
        play->shake_t = RandomF32(&play->random, 3.9, 9.2);

        play->shake += 0.4 * RandomUnilateral(&play->random);
        play->shake  = Min(play->shake, 1.0f);
    }
    else if (JustPressed(input->keys[Key_O])) {
        play->shake += 0.3f;
        play->shake = Min(play->shake, 1.0f);
    }

    v2 shake_offset = play->shake_offset * (play->shake * play->shake) * RandomBilateral(&play->random);
    f32 shake_angle = play->shake_angle  * (play->shake * play->shake) * RandomBilateral(&play->random);

    if (play->debug_camera_enabled) {
        SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), play->debug_camera_p);
    }
    else if (IsPressed(input->keys[Key_P])) {
        SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(play->camera_p, 150));
    }
    else {
        m4x4 zrot = ZRotation(shake_angle);

        v3 x_axis = GetColumn(zrot, 0);
        v3 y_axis = GetColumn(zrot, 1);
        v3 z_axis = GetColumn(zrot, 2);

        SetCameraTransform(batch, 0, x_axis, y_axis, z_axis, V3(play->camera_p + shake_offset, 10));
    }

    v2 world_dim = V2(WORLD_X_SIZE * WORLD_TILE_SIZE, WORLD_Y_SIZE * WORLD_TILE_SIZE);

    // This clamps the player camera to the bounds of the world
    //
    rect3 camera_rect = GetCameraFrustum(&batch->game_tx);
    rect2 screen_bounds;
    screen_bounds.min = camera_rect.min.xy;
    screen_bounds.max = camera_rect.max.xy;

    v2 screen_dim;
    screen_dim.x = (camera_rect.max.x - camera_rect.min.x);
    screen_dim.y = (camera_rect.min.y - camera_rect.max.y); // Inverted because -y up

    screen_dim *= 1.02; // @Hack: The sizes are slightly off so upscale them slightly

    v2 p_min, p_max;

    p_min.x = (0.5f * screen_dim.x - (0.5f * WORLD_TILE_SIZE));
    p_min.y = (0.5f * screen_dim.y - (0.5f * WORLD_TILE_SIZE));

    p_max.x = world_dim.x - (0.5f * screen_dim.x + (0.5f * WORLD_TILE_SIZE));
    p_max.y = world_dim.y - (0.5f * screen_dim.y + (0.5f * WORLD_TILE_SIZE));

    v2 target_p = player->p;

    target_p.x = Clamp(target_p.x, p_min.x, p_max.x);
    target_p.y = Clamp(target_p.y, p_min.y, p_max.y);

    // Update camera to move towards the target p. This is ususally the player's position
    //
    v2 ddp = CAMERA_STIFFNESS * (target_p - play->camera_p) - CAMERA_DAMPING * play->camera_dp;
    play->camera_p  += (0.5f * ddp * dt * dt) + (play->camera_dp * dt);
    play->camera_dp += (ddp * dt);

    // Clear screen
    //
    DrawClear(batch, V4(0.01f, 0.01f, 0.01f, 1.0f));

    UpdatePlayer(play, player, input, state);

    // @Debug: Showing player movement over time
    //
    play->last_p[play->next_last_p] = player->p;
    play->count += 1;
    play->next_last_p += 1;
    play->next_last_p %= ArraySize(play->last_p);

    // Draw World
    //
    v2 tile_dim = V2(WORLD_TILE_SIZE, WORLD_TILE_SIZE);
    for (u32 y = 0; y < WORLD_Y_SIZE; ++y) {
        for (u32 x = 0; x < WORLD_X_SIZE; ++x) {
            Tile *tile = &play->tiles[(y * WORLD_Y_SIZE) + x];
            if (tile->type == Tile_Air) { continue; }
            if (tile->type == Tile_Exit) {
                v2 world_p = V2(tile->grid_p) * tile_dim;
                DrawQuad(batch, {0}, world_p, tile_dim,0, V4(0,1,0,1));
                continue;
            }
            if (tile->type == Tile_Entrance) {
                v2 world_p = V2(tile->grid_p) * tile_dim;
                DrawQuad(batch, {0}, world_p, tile_dim,0, V4(0,0,1,1));
                continue;
            }

            v2 world_p = V2(tile->grid_p) * tile_dim;
            DrawQuad(batch, tile->image, world_p, tile_dim);
        }
    }

    // @Debug: Draw the player movement quads
    //
    u32 count = Min(play->count, ArraySize(play->last_p));
    for (u32 it = 0; it < count; ++it) {
        DrawQuad(batch, { 0 }, play->last_p[it], V2(0.05, 0.05), 0, V4(0, 1, 0, 1));
    }

    // Draw player
    //
    Sprite_Animation *anim = &player->animations[player->cur_anim];
    v2 visual_p = player->p + player->visual_offset;
    DrawAnimation(batch, anim, visual_p, V2(player->facing, 1) * player->visual_dim);

    // @Debug: Player hitbox outline
    //
    //DrawQuadOutline(batch, player->p, player->dim, 0, V4(1, 0, 0, 1), 0.01);

    if (player->flags & Player_Attacking) {
        v2 anim_p = player->p + V2(player->facing * 0.15, 0.03);
        Sprite_Animation *attack_anim = &player->attack_anim;
        DrawAnimation(batch, attack_anim, anim_p, V2(player->facing * 0.6, 0.4));

        u32 total_frames = (attack_anim->rows * attack_anim->cols);
        if (attack_anim->current_frame == total_frames - 1) {
            player->flags &= ~Player_Attacking;
            attack_anim->current_frame = 0;
            attack_anim->time = 0;

            player->drill_p = anim_p;
        }
    }
    else {
        v2 drill_dim = V2(0.3 * player->facing, 0.13);
        if (player->flags & Player_Drilling) {
            drill_dim.y = 0.3f;

            DrawAnimation(batch, &player->drill_anim, player->drill_p - V2(0, 0.05), drill_dim);
        }
        else {
            f32 angle = (player->facing * Degrees(30));

            Image_Handle drill = GetImageByName(&state->assets, "drill");
            DrawQuad(batch, drill, player->drill_p, drill_dim, angle);
        }
    }

    v2 enemy_dim = V2(0.5, 0.5);
    for (u32 it = 0; it < play->enemy_count; ++it) {
        Enemy *enemy = &play->enemies[it];

        UpdateEnemy(play, enemy, dt);
        DrawAnimation(batch, &enemy->anim, enemy->p - V2(0, 0.11f), V2(enemy->x_scale * enemy_dim.x, enemy_dim.y));
    }

    UpdateRenderParticles(batch, play, dt);

    // Update and draw the follower birds
    //
    for (u32 it = 0; it < ArraySize(player->birds); ++it) {
        Bird_Follower *bird = &player->birds[it];

        // Move toward the player
        //
        v2 player_bird_dir = (player->p + (V2(player->facing, 1) * bird->offset)) - bird->p;

        ddp = bird->cp * (player_bird_dir) - bird->vp * bird->dp;
        bird->p  += (0.5f * ddp * dt * dt) + (bird->dp * dt);
        bird->dp += (ddp * dt);

        bird->dir_timer -= input->delta_time;

        // Make the bird face the player direction if it is close enough
        //
        f32 x_scale = bird->x_scale;
        if (Length(ddp) < 0.05f) {
            x_scale = player->facing;
        }
        else if (bird->dir_timer <= 0) {
            // Otherwise if time is up choose a random direction to flip
            //
            bird->x_scale   = (RandomU32(&play->random, 0, 10) > 5) ? -1 : 1;
            bird->dir_timer = RandomF32(&play->random, 1, 3);
        }

        // Draw the bird texture
        //
        DrawQuad(batch, bird->image, bird->p, V2(0.3 * bird->x_scale, 0.3) * 0.4);
    }
    if (IsPressed(input->keys[Key_Enter])) {
        ModeMiniGame(state);
    }
}

function void UpdatePlayer(Mode_Play *play, Player *player, Input *input, Game_State *state) {
    f32 dt = input->delta_time;

    b32 attacking = (player->flags & Player_Attacking);
    b32 on_ground = (player->flags & Player_OnGround);

    f32 gravity = (2 * PLAYER_MAX_JUMP_HEIGHT) / (PLAYER_JUMP_APEX_TIME * PLAYER_JUMP_APEX_TIME);
    v2 ddp      = V2(0, gravity);

    // Attempt to jump. We will buffer this for an amount of time so if the player presses jump
    // slightly before hitting the ground it will still count
    //
    if (JustPressed(input->keys[Key_K])) {
        player->last_jump_time = input->time;
    }

    Sprite_Animation *anim = &player->animations[player->cur_anim];

    UpdateAnimation(anim, dt);
    UpdateAnimation(&player->drill_anim, dt);

    if (attacking) {
        UpdateAnimation(&player->attack_anim, dt);
    }

    // Move left
    //
    if (IsPressed(input->keys[Key_A])) {
        ddp.x = on_ground ? -PLAYER_MOVE_SPEED : -PLAYER_AIR_STRAFE_SPEED;

        player->cur_anim = PlayerAnimation_Run;
        player->facing   = -1;
    }

    // Move right
    //
    if (IsPressed(input->keys[Key_D])) {
        ddp.x = on_ground ? PLAYER_MOVE_SPEED : PLAYER_AIR_STRAFE_SPEED;

        player->cur_anim = PlayerAnimation_Run;
        player->facing   = 1;
    }

    // If neither left or right were pressed apply damping to the player
    //
    if (IsZero(ddp.x)) {
        player->dp.x     *= (1.0f / (1 + (PLAYER_DAMPING * dt)));
        player->cur_anim  = PlayerAnimation_Idle;
    }

    if ((input->time - player->last_jump_time) <= PLAYER_JUMP_BUFFER_TIME) {
        b32 double_jump = (player->flags & Player_DoubleJump);

        if (on_ground || (input->time - player->last_on_ground_time) <= PLAYER_COYOTE_TIME || double_jump) {
            if (double_jump) {
                gravity = (2 * PLAYER_MAX_DOUBLE_JUMP_HEIGHT) / (PLAYER_JUMP_APEX_TIME * PLAYER_JUMP_APEX_TIME);

                player->dp.y   = -Sqrt(2 * gravity * PLAYER_MAX_DOUBLE_JUMP_HEIGHT);
                player->flags &= ~Player_DoubleJump;
            }
            else {
                player->dp.y   = -Sqrt(2 * gravity * PLAYER_MAX_JUMP_HEIGHT);
                player->flags |= Player_DoubleJump;
            }

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
    if (Abs(player->dp.x) > PLAYER_MAX_SPEED_X) {
        player->dp.x *= (PLAYER_MAX_SPEED_X / Abs(player->dp.x));
    }

    // Limit y only in downward direction to prevent jumps from being cut short
    //
    if (player->dp.y > PLAYER_MAX_SPEED_Y) {
        player->dp.y *= (PLAYER_MAX_SPEED_Y / player->dp.y);
    }

    // Drill
    //
    if (IsPressed(input->keys[Key_L]) && on_ground) {
        if (!(player->flags & Player_Drilling)) {
            // TODO Drill noise
        }

        player->flags |= Player_Drilling;
    }
    else if (player->flags & Player_Drilling) {
        player->flags &= ~Player_Drilling;
        // TODO stop Drill noise
    }

    // Attack
    //
    if (!attacking && JustPressed(input->keys[Key_J])) {
        player->flags |= Player_Attacking;
    }

    v2 drill_target = player->p + V2(player->facing * 0.12, 0.1f);

    v2 drill_ddp = (800 * (drill_target - player->drill_p)) - (25 * player->drill_dp);
    player->drill_p  += (0.5f * drill_ddp * dt * dt) + (player->drill_dp * dt);
    player->drill_dp += (drill_ddp * dt);

    // Calculate the drill collision box
    //
    v2 drill_pos = player->p + V2(0.08 * player->facing, -0.15f); // This is not visual position
    v2 drill_dim = V2(0.3, 0.4);

    rect2 drill_r;
    drill_r.min = drill_pos - (0.5f * drill_dim);
    drill_r.max = drill_pos + (0.5f * drill_dim);

    Tile *collision_tiles[9] = {};
    u32 tile_count = GetCloseTiles(player->p, play->tiles, collision_tiles);

    player->flags &= ~Player_OnGround;

    // Calculate the player collision box
    //
    rect2 player_r;
    player_r.min = player->p - (0.5f * player->dim);
    player_r.max = player->p + (0.5f * player->dim);

    // Process collisions
    //
    v2 tile_dim = V2(WORLD_TILE_SIZE, WORLD_TILE_SIZE);

    u32 hits = 0;
    if (player->flags & Player_Drilling) {
        for (u32 it = 0; it < tile_count; ++it) {
            Tile *tile = collision_tiles[it];
            if (tile->type < 0) { continue; }
            if (tile->grid_p.x == 0 || tile->grid_p.y == 0) { continue; }
            if ((tile->grid_p.x + 1) == WORLD_X_SIZE || (tile->grid_p.y + 1) == WORLD_Y_SIZE) { continue; }

            v2 tile_p = V2(tile->grid_p) * tile_dim;

            rect2 tile_r;
            tile_r.min = tile_p - (0.5f * tile_dim);
            tile_r.max = tile_p + (0.5f * tile_dim);

            v2 overlap;
            overlap.x = Min(drill_r.max.x, tile_r.max.x) - Max(drill_r.min.x, tile_r.min.x);
            overlap.y = Min(drill_r.max.y, tile_r.max.y) - Max(drill_r.min.y, tile_r.min.y);

            if (overlap.x >= 0 && overlap.y >= 0) {
                if (overlap.x > 0) {
                    tile->drill_time += dt;

                    if (tile->drill_time > TILE_DRILL_TIME) {
                        tile->type = Tile_Air;

                        for (u32 p = 0; p < 4; ++p) {
                            SpawnDrillDebris(play, &state->assets, drill_pos + V2(player->facing * drill_dim.x, 0.19f));
                        }
                    }
                    else {
                        hits += 1;

                        if (!player->drill_hit_sound) {
                            Sound_Handle drill      = GetSoundByName(&state->assets, "drillbrrr");
                            player->drill_hit_sound = PlaySound(&state->audio_state, drill, PlayingSound_Looped);
                        }
                    }
                }
            }
            else {
                tile->drill_time = 0;
            }
        }

        if (hits != 0) {
            if (play->shake <= 0.3) {
                play->shake += (0.3f - play->shake);
            }

            player->drill_particle_time -= dt;
            if (player->drill_particle_time <= 0.0f) {
                u32 particle_count = RandomU32(&play->random, 2, 8);
                for (u32 p = 0; p < particle_count; ++p) {
                    SpawnDrillDebris(play, &state->assets, drill_pos + V2(player->facing * 0.55 * drill_dim.x, 0.19));
                }

                player->drill_particle_time = 0.2f;
            }
        }
        else {
            if (play->shake <= 0.18f) {
                play->shake += (0.18f - play->shake);
            }
        }
    }

    if (hits == 0 && player->drill_hit_sound) {
        StopSound(&state->audio_state, player->drill_hit_sound);
        player->drill_hit_sound = 0;
    }

    for (u32 it = 0; it < tile_count; ++it) {
        Tile *tile = collision_tiles[it];
        if (tile->type < 0) { continue; }

        v2 tile_p = V2(tile->grid_p) * tile_dim;

        rect2 tile_r;
        tile_r.min = tile_p - (0.5f * tile_dim);
        tile_r.max = tile_p + (0.5f * tile_dim);

        v2 overlap;
        overlap.x = Min(player_r.max.x, tile_r.max.x) - Max(player_r.min.x, tile_r.min.x);
        overlap.y = Min(player_r.max.y, tile_r.max.y) - Max(player_r.min.y, tile_r.min.y);

        if (overlap.x >= 0 && overlap.y >= 0) {
            v2 dir = (player->p - tile_p);

            if (overlap.x < overlap.y) {
                if (overlap.x > 0) {
                    f32 sign      = Sign(dir.x);
                    player->dp.x  = 0;
                    player->p.x  += (overlap.x * sign);
                }
            }
            else {
                f32 sign     = Sign(dir.y);
                player->p.y += (overlap.y * sign);

                if (sign < 0) {
                    player->flags |=  Player_OnGround;
                    player->flags &= ~Player_DoubleJump;

                    player->dp.y = 0;
                    player->last_on_ground_time = input->time;
                }
                else {
                    player->dp.y  = 0;
                    player->dp.y += (ddp.y * dt);
                }
            }

            // Update the player hitbox to incorporate the new position
            //
            player_r.min = player->p - (0.5f * player->dim);
            player_r.max = player->p + (0.5f * player->dim);
        }
    }
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

function void SpawnParticle(Mode_Play *play, Image_Handle image, v3 p, v3 dp, f32 a, f32 da, f32 s, f32 ds, f32 t) {
    Particle *particle = &play->particle_cache[play->next_particle];
    play->next_particle += 1;
    play->next_particle %= play->max_particles;

    particle->image = image;

    particle->p     = p;
    particle->dp    = dp;

    particle->a     = a;
    particle->da    = da;

    particle->s     = s;
    particle->ds    = ds;

    particle->t     = t;
}

function void SpawnDrillDebris(Mode_Play *play, Asset_Manager *assets, v2 p) {
    v2 dp = V2(RandomBilateral(&play->random), RandomBilateral(&play->random));

    // a and da
    //
    f32 a  = RandomF32(&play->random, 0, Tau32);
    f32 da = RandomUnilateral(&play->random);

    f32 t = RandomF32(&play->random, 0.5f, 1.3f);

    str8 image_names[] = {
        WrapConst("pebble_01"),
        WrapConst("pebble_02"),
        WrapConst("spark_01"),
        WrapConst("spark_02"),
        WrapConst("spark_03"),
        WrapConst("spark_04")
    };

    u32 choice = NextRandom(&play->random) % ArraySize(image_names);
    Image_Handle image = GetImageByName(assets, image_names[choice]);

    f32 s, ds;
    if (choice != 0) {
        s  = 0.08f;
        ds = -0.18f * RandomF32(&play->random, 0.5f, 1.0f);
    }
    else {
        s  = 0.1 * RandomF32(&play->random, 0.8f, 1.0f);
        ds = 0;
    }

    // @Todo: Image handle
    //
    SpawnParticle(play, image, V3(p), V3(dp), a, da, s, ds, t);
}

function void UpdateRenderParticles(Draw_Batch *batch, Mode_Play *play, f32 dt) {
    for (u32 it = 0; it < play->max_particles; ++it) {
        Particle *p = &play->particle_cache[it];
        if (p->t <= 0.0f) { continue; }

        v3 ddp = V3(0.0f, 2.3f, 0.0f);

        p->p  += (p->dp * dt);
        p->dp += (ddp * dt);

        p->a += (p->da * dt);
        p->s += (p->ds * dt);

        p->t -= dt;

        DrawQuad(batch, p->image, p->p, V2(p->s, p->s), p->a);
    }
}

function b32 CanTraverse(Mode_Play *play, v2 p, f32 xdir) {
    b32 result = false;

    v2s grid_p;
    grid_p.x = cast(s32) ((p.x / WORLD_TILE_SIZE) + 0.5f);
    grid_p.y = cast(s32) ((p.y / WORLD_TILE_SIZE) + 0.5f);

    grid_p.x += xdir;
    grid_p.y += 1; // Tile below for standing on

    if (IsValidTile(grid_p)) {
        Tile *tile = &play->tiles[(grid_p.y * WORLD_Y_SIZE) + grid_p.x];
        result = (tile->type == Tile_Ground);
    }

    return result;
}

function void UpdateEnemy(Mode_Play *play, Enemy *enemy, f32 dt) {
    UpdateAnimation(&enemy->anim, dt);

    f32 gravity = (2 * PLAYER_MAX_JUMP_HEIGHT) / (PLAYER_JUMP_APEX_TIME * PLAYER_JUMP_APEX_TIME);
    v2 ddp = V2(0, gravity);

    enemy->p  += (enemy->dp * dt);
    enemy->dp += (ddp * dt);

    if (!CanTraverse(play, enemy->p, enemy->dp.x)) {
        enemy->dp.x = 0;
    }

    enemy->decision_wait -= dt;
    if (enemy->decision_wait < 0) {
        u32 move_decision = RandomU32(&play->random, 0, 3);
        switch (move_decision) {
            case 0:
            case 2: {
                if (CanTraverse(play, enemy->p, 1)) {
                    enemy->dp.x    = 1;
                    enemy->x_scale = -1;
                }
                else {
                    enemy->dp.x    = -1;
                    enemy->x_scale = 1;
                }
            }
            break;
            case 1:
            case 3: {
                if (CanTraverse(play, enemy->p, -1)) {
                    enemy->dp.x    = -1;
                    enemy->x_scale = 1;
                }
                else {
                    enemy->dp.x    = 1;
                    enemy->x_scale = -1;
                }
            }
        }

        enemy->decision_wait = RandomF32(&play->random, 2.2f, 9.1f);
    }

    // Check collision with player
    //
    Player *player = &play->player;

    v2 enemy_dim = V2(0.15f, 0.3f);
    v2 tile_dim  = V2(WORLD_TILE_SIZE, WORLD_TILE_SIZE);

    rect2 enemy_r;
    enemy_r.min = enemy->p - 0.5f * V2(1.8, 1) * enemy_dim;
    enemy_r.max = enemy->p + 0.5f * V2(1.8, 1) * enemy_dim;

    rect2 player_r;
    player_r.min = player->p - (0.5f * player->dim);
    player_r.max = player->p + (0.5f * player->dim);

    if (player->flags & Player_Attacking) {
        v2 attack_dim = V2(0.6, 0.4);
        v2 anim_p = player->p + V2(player->facing * 0.15, 0.03);

        player_r.min = anim_p - (0.5f * attack_dim);
        player_r.max = anim_p + (0.5f * attack_dim);
    }

    {
        v2 overlap;
        overlap.x = Min(enemy_r.max.x, player_r.max.x) - Max(enemy_r.min.x, player_r.min.x);
        overlap.y = Min(enemy_r.max.y, player_r.max.y) - Max(enemy_r.min.y, player_r.min.y);

        if (overlap.x >= 0 && overlap.y >= 0) {
            // @Todo: Start minigame here
            //
            //
        }
    }

    // Check collision with world
    //
    Tile *collision_tiles[9];
    u32 tile_count = GetCloseTiles(enemy->p, play->tiles, collision_tiles);

    for (u32 it = 0; it < tile_count; ++it) {
        Tile *tile = collision_tiles[it];
        if (tile->type < 0) { continue; }

        v2 tile_p = V2(tile->grid_p) * tile_dim;

        rect2 tile_r;
        tile_r.min = tile_p - (0.5f * tile_dim);
        tile_r.max = tile_p + (0.5f * tile_dim);

        v2 overlap;
        overlap.x = Min(enemy_r.max.x, tile_r.max.x) - Max(enemy_r.min.x, tile_r.min.x);
        overlap.y = Min(enemy_r.max.y, tile_r.max.y) - Max(enemy_r.min.y, tile_r.min.y);

        if (overlap.x >= 0 && overlap.y >= 0) {
            v2 dir = (enemy->p - tile_p);

            if (overlap.x < overlap.y) {
                if (overlap.x > 0) {
                    f32 sign      = Sign(dir.x);
                    enemy->dp.x *= -1;
                    enemy->p.x  += (overlap.x * sign);
                }
            }
            else {
                f32 sign     = Sign(dir.y);
                enemy->p.y += (overlap.y * sign);

                if (sign < 0) {
                    enemy->dp.y = 0;
                }
                else {
                    // This shouldn't really happen but for completeness
                    //
                    enemy->dp.y  = 0;
                    enemy->dp.y += (ddp.y * dt);
                }
            }

            // Update the enemy hitbox to incorporate the new position
            //
            enemy_r.min = enemy->p - (0.5f * enemy_dim);
            enemy_r.max = enemy->p + (0.5f * enemy_dim);
        }
    }
}
