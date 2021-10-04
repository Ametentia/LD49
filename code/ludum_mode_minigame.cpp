#include <stdio.h>

function void ModeMiniGame(Game_State *state){
    state->mode = GameMode_MiniGame;
    Mode_MiniGame *minigame = AllocType(&state->mode_arena,Mode_MiniGame);
    state->play->minigame = minigame;
    Random *random = &state->play->random;
    minigame->type = (MiniGameType)RandomU32(random, 0, 2);
    minigame->type = MiniGame_RockFall;

    switch(state->play->minigame->type) {
        case MiniGame_BinaryCount: {
                minigame->binary.target = RandomU32(random, 1, 16);
                Image_Handle walk_sheet = GetImageByName(&state->assets, "Walking_sheet");
                Initialise(&minigame->binary.animation, walk_sheet, 1, 20, 1.0/32.0);
                minigame->binary.x = 5;
                minigame->binary.y = 2;
                minigame->binary.x_dir = 1;
            }
            break;
        case MiniGame_RockFall: {
                minigame->rockFall.free_space = RandomU32(random, 0, 5); 
                minigame->rockFall.free_space2 = RandomU32(random, 0, 5); 
                while(minigame->rockFall.free_space == minigame->rockFall.free_space2) {
                    minigame->rockFall.free_space = RandomU32(random, 0, 5); 
                }
                minigame->rockFall.x = 2;
                minigame->rockFall.drop_time = 4;
                minigame->rockFall.time_passed = 0;
                minigame->rockFall.x_dir = 1;
                Image_Handle walk_sheet = GetImageByName(&state->assets, "Walking_sheet");
                Initialise(&minigame->rockFall.animation, walk_sheet, 1, 20, 1.0/32.0);
            }
            break;
        case MiniGame_IceSkating: {
                MiniGamePlayer *player = &minigame->ice.player;
                player->pos = V2(0,0);
                player->dim = V2(0.3,0.3);
                player->xOnGrid = 0;
                player->yOnGrid = 0;
                player->x_scale = 1;
                Image_Handle walk_sheet = GetImageByName(&state->assets, "Walking_sheet");
                Initialise(&player->animation, walk_sheet, 1, 20, 1.0/64.0);
                BuildMap(state);
            }
        break;
    }
}

function void UpdateRenderModeMiniGame(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer) {
    Draw_Batch _batch = {};
    Draw_Batch *batch = &_batch;

    Initialise(batch, &state->assets, renderer_buffer);
    SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(0, 0, 13));

    DrawClear(batch, V4(0.01f, 0.01f, 0.01f, 1.0f));
    switch(state->play->minigame->type) {
        case MiniGame_BinaryCount: {
                UpdateRenderBinaryCount(state, input, batch);
            }
            break;
        case MiniGame_IceSkating: {
                UpdateRenderIceSkating(state, input, batch);
            }
            break;
        case MiniGame_RockFall: {
                UpdateRenderRockFall(state, input, batch);
            }
            break;
    }
}

function void UpdateRenderRockFall(Game_State *state, Input *input, Draw_Batch *batch) {
    f32 dt = input->delta_time;
    RockFall *minigame = &state->play->minigame->rockFall;
    Image_Handle back = GetImageByName(&state->assets, "background_01");
    DrawQuad(batch, back, V2(0,0), 8, 0, V4(1,1,1,1));
    f32 offset_x = -(0.3f*5)/2 ;
    minigame->time_passed += dt;
    for(u32 i = 0; i < 5; i++) {
        for(u32 j = 0; j < 9; j++) {
            v4 colour = V4(0.4f,0.4f,0.4f,1);
            Image_Handle h = GetImageByName(&state->assets, "ground_02");
            DrawQuad(batch, h, V2(0.15 + offset_x+i*0.3, 0.15-(j*0.3)), V2(0.3,0.3), 0, colour);
        }
    }
    if(minigame->time_passed < minigame->drop_time) {
        if (JustPressed(input->keys[Key_D]) && minigame->x < 4) {
            minigame->x += 1;
            minigame->x_dir = 1.0;
        }
        if (JustPressed(input->keys[Key_A]) && minigame->x > 0) {
            minigame->x -= 1;
            minigame->x_dir = -1.0;
        }
        UpdateAnimation(&minigame->animation, dt);
    }
    else {
        minigame->animation.current_frame = 0;
        f32 progress = Min((minigame->time_passed-minigame->drop_time)*2, 2);
        for(u32 i = 0; i < 5; i++) {
            if(i == minigame->free_space || i == minigame->free_space2)
                continue;
            Image_Handle h = GetImageByName(&state->assets, "pebble_01");
            DrawQuad(batch, h, V2(0.15 + offset_x+i*0.3, Lerp(-2.0f, -1.05f, progress)), V2(0.3,0.3), progress*3, V4(1,1,1,1));
            if(progress == 2) {
                state->mode = GameMode_Play;
            }
        }
    }
    DrawAnimation(batch, &minigame->animation, V2(0.15 + offset_x+(minigame->x*0.3), 0.15), V2(0.3*minigame->x_dir, 0.3), 0, V4(1,1,1,1));
}

function void UpdateRenderBinaryCount(Game_State *state, Input *input, Draw_Batch *batch) {
    f32 dt = input->delta_time;
    MiniGameBinary *minigame = &state->play->minigame->binary;
    f32 offset_x = -(0.3f*9)/2 ;
    u32 changed_pos = 0;
    Image_Handle back = GetImageByName(&state->assets, "background_01");
    DrawQuad(batch, back, V2(0,0), 8, 0, V4(1,1,1,1));
    if (JustPressed(input->keys[Key_D]) && minigame->x < 8) {
        minigame->x += 1;
        minigame->x_dir = 1.0;
        changed_pos = 1;
    }
    if (JustPressed(input->keys[Key_A]) && minigame->x > 0) {
        minigame->x -= 1;
        minigame->x_dir = -1.0;
        changed_pos = 1;
    }
    if (JustPressed(input->keys[Key_W]) && minigame->y > 0) {
        minigame->y -= 1;
        changed_pos = 1;
    }
    if (JustPressed(input->keys[Key_S]) && minigame->y < 3) {
        minigame->y += 1;
        changed_pos = 1;
    }
    char numbers_back[10][20] = {
        "number_0_background",
        "number_1_background",
        "number_2_background",
        "number_3_background",
        "number_4_background",
        "number_5_background",
        "number_6_background",
        "number_7_background",
        "number_8_background",
        "number_9_background"
    };
    char numbers_fore[10][20] = {
        "number_0_foreground",
        "number_1_foreground",
        "number_2_foreground",
        "number_3_foreground",
        "number_4_foreground",
        "number_5_foreground",
        "number_6_foreground",
        "number_7_foreground",
        "number_8_foreground",
        "number_9_foreground"
    };
    u32 text_aim = minigame->target;
    v2 text_pos = V2(-1,-1.2);
    if(text_aim > 9){
        v2 tens_pos = V2(-1.3,-1.2);
        Image_Handle fore = GetImageByName(&state->assets, numbers_fore[1]);
        Image_Handle back = GetImageByName(&state->assets, numbers_back[1]);
        DrawQuad(batch, back, tens_pos , V2(0.5,0.5), 0, V4(1,1,1,1));
        DrawQuad(batch, fore, tens_pos, V2(0.5,0.5), 0, V4(1,0,0,1));
        u32 remainder = text_aim-10;
        fore = GetImageByName(&state->assets, numbers_fore[remainder]);
        back = GetImageByName(&state->assets, numbers_back[remainder]);
        DrawQuad(batch, back, text_pos, V2(0.5,0.5), 0, V4(1,1,1,1));
        DrawQuad(batch, fore, text_pos, V2(0.5,0.5), 0, V4(1,0,0,1));
    } else {
        Image_Handle fore = GetImageByName(&state->assets, numbers_fore[text_aim]);
        Image_Handle back = GetImageByName(&state->assets, numbers_back[text_aim]);
        DrawQuad(batch, back, text_pos, V2(0.5,0.5), 0, V4(1,1,1,1));
        DrawQuad(batch, fore, text_pos, V2(0.5,0.5), 0, V4(1,0,0,1));
    }
    // I couldn't be arsed okay
    text_aim = minigame->setting;
    text_pos = V2(1.3,-0.7);
    if(text_aim > 9){
        v2 tens_pos = V2(1,-0.7);
        Image_Handle fore = GetImageByName(&state->assets, numbers_fore[1]);
        Image_Handle back = GetImageByName(&state->assets, numbers_back[1]);
        DrawQuad(batch, back, tens_pos , V2(0.5,0.5), 0, V4(1,1,1,1));
        DrawQuad(batch, fore, tens_pos, V2(0.5,0.5), 0, V4(1,0,0,1));
        u32 remainder = text_aim-10;
        fore = GetImageByName(&state->assets, numbers_fore[remainder]);
        back = GetImageByName(&state->assets, numbers_back[remainder]);
        DrawQuad(batch, back, text_pos, V2(0.5,0.5), 0, V4(1,1,1,1));
        DrawQuad(batch, fore, text_pos, V2(0.5,0.5), 0, V4(1,0,0,1));
    } else {
        Image_Handle fore = GetImageByName(&state->assets, numbers_fore[text_aim]);
        Image_Handle back = GetImageByName(&state->assets, numbers_back[text_aim]);
        DrawQuad(batch, back, text_pos, V2(0.5,0.5), 0, V4(1,1,1,1));
        DrawQuad(batch, fore, text_pos, V2(0.5,0.5), 0, V4(1,0,0,1));
    }
    for(u32 i = 0; i < 9; i++) {
        for(u32 j = 0; j < 4; j++) {
            v4 colour = V4(0.4f,0.4f,0.4f,1);
            Image_Handle h = GetImageByName(&state->assets, "ground_01");
            if(i%2 == 1 && j == 1) {
                if(changed_pos && i==minigame->x && j==minigame->y) {
                    minigame->setting ^= (8>>i/2);
                    Sound_Handle music_handle = GetSoundByName(&state->assets, "clickclack");
                    PlaySound(&state->audio_state, music_handle);
                }
                u32 red_green = (minigame->setting & 8>>i/2) << i/2;
                colour = V4(!red_green,red_green,0,1);
                h = {0};
            }
            DrawQuad(batch, h, V2(0.15 + offset_x+i*0.3, (0.3*j) - 0.15), V2(0.3,0.3), 0, colour);
        }
    }
    UpdateAnimation(&minigame->animation, dt);
    DrawAnimation(batch, &minigame->animation, V2(0.15 + offset_x+(minigame->x*0.3), (0.3*minigame->y) - 0.15), V2(0.3*minigame->x_dir, 0.3), 0, V4(1,1,1,1));
    if(minigame->setting == minigame->target) {
        state->mode = GameMode_Play;
    }
}

function void UpdateRenderIceSkating(Game_State *state, Input *input, Draw_Batch *batch) {
    MiniGameIceSkating *minigame = &state->play->minigame->ice;

    MiniGamePlayer *player = &minigame->player;
    UpdateMGPlayer(player, input, minigame, state);

    DrawQuad(batch, {0}, V2(0,0),V2(6.0,3.5), 0, V4(60/255.0, 5/255.0, 5/255.0,1.0));

    for(u32 i = 0; i < minigame->height*minigame->width; i++){
        DrawQuad(batch, minigame->tiles[i].asset, minigame->tiles[i].p, minigame->tiles[i].dim);
    }

    DrawAnimation(batch, &player->animation, player->pos,V2(player->x_scale,1)*player->dim, 0, V4(1,1,1,1));
}

function void UpdateMGPlayer(MiniGamePlayer *player, Input *input, MiniGameIceSkating *minigame, Game_State *state){
    f32 dt = input->delta_time;
    UpdateAnimation(&player->animation, dt);

    // Move left
    //
    if (JustPressed(input->keys[Key_A])) {
        if(player->xOnGrid > 0 && minigame->tiles[player->xOnGrid + (minigame->height*player->yOnGrid)].type != 0){
            player->xOnGrid--;
            player->x_scale = -1;
        }
    }
    // Move right
    //
    if (JustPressed(input->keys[Key_D])) {
        if(player->xOnGrid < minigame->width && minigame->tiles[player->xOnGrid + (minigame->height*player->yOnGrid)].type != 0){
            player->xOnGrid++;
            player->x_scale = 1;
        }
    }
    // Move up
    //
    if (JustPressed(input->keys[Key_W]) ) {
        if(player->yOnGrid > 0 && minigame->tiles[player->xOnGrid + (minigame->height*player->yOnGrid)].type != 0){
            player->yOnGrid--;
        }
    }
    // Move down
    //
    if (JustPressed(input->keys[Key_S])) {
        if(player->yOnGrid < minigame->height && minigame->tiles[player->xOnGrid + (minigame->height*player->yOnGrid)].type != 0){
            player->yOnGrid++;
        }
    }
    player->pos =  minigame->tiles[player->xOnGrid + (minigame->width*player->yOnGrid)].p;
}

function void BuildMap(Game_State *state){
    Mode_Play *play = state->play;
    Random *random = &play->random;
    MiniGameIceSkating *minigame = &play->minigame->ice;
    minigame->heights[0] = 3;
    minigame->heights[1] = 2;
    minigame->heights[2] = 3;
    minigame->heights[3] = 2;
    minigame->widths[0] = 3;
    minigame->widths[1] = 4;
    minigame->widths[2] = 4;
    minigame->widths[3] = 2;
    int tileNumbers[] = {
        1,1,1,1,1,1,1,1,1,-1,-1,-1,
        1,0,1,1,1,1,0,1,-1,-1,-1,-1,
        1,1,1,1,1,0,1,1,1,1,1,0,
        1,1,0,1,1,1,1,0,-1,-1,-1,-1
    };
    u32 r = RandomU32(random, 0,3);
    minigame->height = minigame->heights[r];
    minigame->width = minigame->widths[r];

    for(u32 i = 0; i<minigame->height;i++){
        for(u32 j = 0; j < minigame->width; j++){
            MiniGameTile *t = &minigame->tiles[(i*minigame->width)+j];
            t->type = (MiniGameTileType)tileNumbers[(i*minigame->width)+j+(r*12)];
            t->dim = V2(0.3,0.3);
            t->walkedOn = false;
            t->p = V2(0.31*j, 0.31*i);
            t->asset = GetImageByName(&state->assets,t->type == 1 && !t->walkedOn ? "ground_01" : "ground_02");
        }
    }
}
