#include <stdio.h>

function void ModeMiniGame(Game_State *state){
    state->mode = GameMode_MiniGame;
    Mode_MiniGame *minigame = AllocType(&state->mode_arena,Mode_MiniGame);
    state->play->minigame = minigame;

    MiniGamePlayer *player = &minigame->player;
    player->pos = V2(0,0);
    player->dim = V2(0.3,0.3);
    player->x_scale = 1;
    Image_Handle walk_sheet = GetImageByName(&state->assets, "Walking_sheet");
    Initialise(&player->animation, walk_sheet, 1, 20, 1.0/64.0);
    BuildMap(state);
}

function void UpdateRenderModeMiniGame(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer) {
    Mode_MiniGame *minigame = state->play->minigame;

    Draw_Batch _batch = {};
    Draw_Batch *batch = &_batch;

    MiniGamePlayer *player = &minigame->player;
    UpdateMGPlayer(player, input, minigame, state);

    Initialise(batch, &state->assets, renderer_buffer);
    SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(0, 0, 13));

    DrawClear(batch, V4(0.01f, 0.01f, 0.01f, 1.0f));
    DrawQuad(batch, {0}, V2(0,0),V2(6.0,3.5), 0, V4(60/255.0, 5/255.0, 5/255.0,1.0));

    for(u32 i = 0; i < minigame->height*minigame->width; i++){
        DrawQuad(batch, minigame->tiles[i].asset, minigame->tiles[i].p, minigame->tiles[i].dim);
    }

    DrawAnimation(batch, &player->animation, player->pos,V2(player->x_scale,1)*player->dim, 0, V4(1,1,1,1));
}

function void UpdateMGPlayer(MiniGamePlayer *player, Input *input, Mode_MiniGame *minigame, Game_State *state){
    f32 dt = input->delta_time;
    UpdateAnimation(&player->animation, dt);

    v2 movementVect = V2(0,0);
    // Move left
    //
    if (IsPressed(input->keys[Key_A])) {
        if(player->pos.x - 0.1 < 0){
            player->pos.x = 0;
        }
        else{
            movementVect.x -= 0.05;
        }
        player->x_scale = -1;
    }
    // Move right
    //
    if (IsPressed(input->keys[Key_D])) {
        if(player->pos.x + 0.1 > (minigame->width-1)*0.31){
            player->pos.x = (minigame->width-1)*0.31;
        }
        else{
            movementVect.x += 0.05;

        }
        player->x_scale = 1;
    }
    // Move up
    //
    if (IsPressed(input->keys[Key_W])) {
        if(player->pos.y - 0.1 < 0){
            player->pos.y = 0;    
        }
        else{
            movementVect.y -= 0.05;
        }
    }
    // Move down
    //
    if (IsPressed(input->keys[Key_S])) {
        if(player->pos.y + 0.1 > (minigame->height-1)*0.31){
            player->pos.y = (minigame->height-1)*0.31;    
        }
        else{
            movementVect.y += 0.05;
        }
    }

    MiniGameTile obstructionTiles[];
    u32 j = 0;
    for(u32 i = 0; i < minigame->width*minigame->height; i++){
        if(minigame->tiles[i].value == 0){
            obstructionTiles[j++] = minigame->tiles[i];
        }
    }
    for(u32 i = 0; i < j; i++){
        if(player->pos.x > obstructionTiles[i].p.x && player->pos.x < obstructionTiles[i].p.x+0.31){

        }
    }
    

    for(u32 i = 0; i < minigame->width*minigame->height; i++){
        if(Length(minigame->tiles[i].p - player->pos) < 0.05){
            minigame->tiles[i].walkedOn = true;
            minigame->tiles[i].asset = GetImageByName(&state->assets,"ground_02");
        }
    }
}

function void BuildMap(Game_State *state){
    Mode_Play *play = state->play;
    Random *random = &play->random;
    Mode_MiniGame *minigame = play->minigame;
    minigame->heights[0] = 3;
    minigame->heights[1] = 2;
    minigame->heights[2] = 3;
    minigame->heights[3] = 2;
    minigame->widths[0] = 3;
    minigame->widths[1] = 4;
    minigame->widths[2] = 4;
    minigame->widths[3] = 2;
    s32 tileNumbers[] = {
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
            t->value = tileNumbers[(i*minigame->width)+j+(r*12)];
            t->dim = V2(0.3,0.3);
            t->walkedOn = false;
            t->p = V2(0.31*j, 0.31*i);
            t->asset = GetImageByName(&state->assets,t->value == 1 && !t->walkedOn ? "ground_01" : "ground_02");
        }
    }
}