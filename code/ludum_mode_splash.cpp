function Move_Type GetOppositeMove(Move_Type move) {
    Move_Type opp;
    switch(move) {
        case MoveType_Up: {
            opp = MoveType_Down;
        }
        break;
        case MoveType_Down: {
            opp = MoveType_Up;
        }
        break;
        case MoveType_Right: {
            opp = MoveType_Left;
        }
        break;
        case MoveType_Left: {
            opp = MoveType_Right;
        }
        break;
    };
    return opp;
}
function Move FindMove(Block blocks[], v2 empty, Move_Type lastMove, Random *rand) {
    Move moves[8] = {};
    u8 moveCount = 0;
    for(u8 i = 0; i < 8; i++) {
        Block block = blocks[i];
        f32 diffX = block.pos.x - empty.x;
        f32 diffY = block.pos.y - empty.y;
        f32 change = Abs(diffX) + Abs(diffY);
        if(change != 1.0) {
            continue;
        }
        moves[moveCount].index = i;
        if(diffX == 1.0) {
            moves[moveCount].move = MoveType_Left;
        }
        else if(diffX == -1.0) {
            moves[moveCount].move = MoveType_Right;
        }
        else if(diffY == -1.0) {
            moves[moveCount].move = MoveType_Down;
        }
        else if(diffY == 1.0) {
            moves[moveCount].move = MoveType_Up;
        }
        if(GetOppositeMove(moves[moveCount].move) != lastMove) {
            moveCount++;
        }
    }
    u32 randNum = RandomU32(rand, 0, moveCount);
    return moves[randNum];
}

function v2 PredictMove(Block blocks[], Move move) {
    v2 pos = blocks[move.index].pos;
    switch(move.move) {
        case MoveType_Up: {
            pos.y -= 1;
        }
        break;
        case MoveType_Down: {
            pos.y += 1;
        }
        break;
        case MoveType_Right: {
            pos.x += 1;
        }
        break;
        case MoveType_Left: {
            pos.x -= 1;
        }
        break;
    };
    return pos;
}
function v2 ApplyMove(Block blocks[], Move move) {
    v2 empty = blocks[move.index].pos;
    blocks[move.index].pos = PredictMove(blocks, move);
    return empty;
}

function void ModeSplash(Game_State *state, Input *input) {
    Reset(&state->mode_arena);

    Mode_Splash *splash = AllocType(&state->mode_arena, Mode_Splash);
    splash->alloc = &state->mode_arena;
    splash->steps = 16;
    splash->time = 0;
    splash->timer = 0;
    splash->move_time = 0.07f;
    splash->fade_start = splash->move_time*(splash->steps+11);
    splash->rand = RandomSeed(input->ticks);
    for(u8 i = 0; i < 8; i++) {
        u8 mid_skipped = i;
        if(i > 3){
            mid_skipped++;
        }
        u8 x = mid_skipped % 3;
        u8 y = mid_skipped / 3;
        splash->blocks[i].pos = V2(x, y);
        splash->blocks[i].blockPos = V2(x, y);
    }
    v2 empty = V2(1, 1);
    Move_Type lastMove = MoveType_None;
    for(u8 i = 0; i < splash->steps; i++) {
        Move move = FindMove(splash->blocks, empty, lastMove, &splash->rand);
        Move reverse = {};
        if(i + 2 == splash->steps) {
            splash->lastPos = splash->blocks[move.index].pos;
        }
        reverse.move = GetOppositeMove(move.move);
        reverse.index = move.index;
        splash->moves[i] = reverse;
        empty = ApplyMove(splash->blocks, move);
        lastMove = move.move;
    }

    state->mode   = GameMode_Splash;
    state->splash = splash;
}

function void DrawLogoSection(Image_Handle image, Draw_Batch *batch, u8 count, u8 x, u8 y, v2 centre, v2 dim, v4 colour) {
    v2 half_dim = 0.5f * dim;
    v2 p0 = centre + V2(-half_dim.x, half_dim.y);
    v2 p1 = centre + -half_dim;
    v2 p2 = centre + V2(half_dim.x, -half_dim.y);
    v2 p3 = centre + half_dim;

    f32 width = 1.0/count;
    f32 height = 1.0/count;
    u32 col = ABGRPack(colour);
    vert3 positions[4] = {
        V3(p0), V2(width*x, height*y), col,
        V3(p1), V2(width*x, height*(y+1)), col,
        V3(p2), V2(width*(x+1), height*(y+1)), col,
        V3(p3), V2(width*(x+1), height*y), col
    };
    DrawQuad(batch, image, positions[0], positions[1], positions[2], positions[3]);
}

function void DrawLogoSection(Image_Handle image, Draw_Batch *batch, u8 count, u8 x, u8 y, v2 centre, v2 dim) {
    DrawLogoSection(image, batch, count, x, y, centre, dim, V4(1,1,1,1));
}

function void UpdateRenderModeSplash(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer) {
    Draw_Batch _batch = {};
    Draw_Batch *batch = &_batch;
    Initialise(batch, &state->assets, renderer_buffer);
    DrawClear(batch, V4(0, 0, 0, 0));
    SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(0, 0, 15));

    Mode_Splash *mode = state->splash;

    f32 dt = input->delta_time;
    mode->time += dt;
    mode->timer += dt;
    if(mode->timer > mode->move_time && mode->steps != 255) {
        Move move = mode->moves[mode->steps];
        ApplyMove(mode->blocks, move);
        mode->steps -= 1;
        mode->timer = 0;
        char sounds[3][7] = {
            "slide0",
            "slide1",
            "slide2"
        };
        Sound_Handle slide = GetSoundByName(&state->assets, sounds[RandomU32(&mode->rand, 0, 2)]);
        PlaySound(&state->audio_state, slide, 0.2f);
    }

    f32 size = 1.1f;
    Image_Handle texture = GetImageByName(&state->assets, "Logo");
    for(u8 i = 0; i < 8; i++) {
        f32 x = mode->blocks[i].pos.x;
        f32 y = mode->blocks[i].pos.y;
        f32 blockX = mode->blocks[i].blockPos.x;
        f32 blockY = mode->blocks[i].blockPos.y;
        if(i == mode->moves[mode->steps].index) {
            v2 prediciton = PredictMove(mode->blocks, mode->moves[mode->steps]);
            f32 alpha = mode->timer/mode->move_time;
            x = Lerp(x, prediciton.x, alpha);
            y = Lerp(y, prediciton.y, alpha);
        }
        DrawLogoSection(texture, batch, 3, blockX, blockY, V2(-size + x * size, size - y*size), V2(size, size));
    }
    if(mode->time > mode->fade_start) {
        Image_Handle ame = GetImageByName(&state->assets, "ametentia");
        if(!mode->done) {
            Sound_Handle snap = GetSoundByName(&state->assets, "snap");
            PlaySound(&state->audio_state, snap, 0.4);
            mode->done = 1;
        }
        f32 diff = mode->time - mode->fade_start;
        DrawLogoSection(
            texture,
            batch,
            3, 1, 1,
            V2(0, 0),
            V2(size, size),
            V4(1,1,1, Min(diff*2, 1))
        );
        DrawQuad(batch, ame, V2(0, 1.9f), V2(4, 0.8f), 0, V4(1,1,1,Min(diff*2, 1)));
    }
    if (IsPressed(input->keys[Key_Space]) || (mode->time - mode->fade_start)>2) {
        ModeMenu(state, &mode->rand);
    }
}
