#include <stdio.h>

function void ModeMiniGame(Game_State *state){
    state->mode = GameMode_MiniGame;
    Mode_MiniGame *minigame = AllocType(&state->mode_arena,Mode_MiniGame);
    state->play->minigame = minigame;
    // DEBUG @DEC Change this to your game
    minigame->type = MiniGame_IceSkating;


    switch(state->play->minigame->type) {
        case MiniGame_BinaryCount: {
                
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
    }
}

function void UpdateRenderBinaryCount(Game_State *state, Input *input, Draw_Batch *batch) {
    Mode_MiniGame *minigame = state->play->minigame;

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
