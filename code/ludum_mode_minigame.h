#if !defined(LUDUM_MODE_MINIGAME_H_)
#define LUDUM_MODE_MINIGAME_H_

enum MiniGameType {
    BinaryCount = 0,
    IceSkating
};

struct MiniGamePlayer{
    f32 x_scale;
    v2 pos;
    v2 dim;
    Sprite_Animation animation;
};

struct MiniGameTile{
    u32 value;
    Image_Handle asset;
    v2 p;
    v2 dim;
    bool walkedOn;
};

struct MiniGameIceSkating {
    MiniGamePlayer player;
    MiniGameTile tiles[12];
    u32 widths[4];
    u32 heights[4];
    u32 width;
    u32 height;
};

struct MiniGameBinary{
    u32 placeholder;
};

struct Mode_MiniGame{
    MiniGameType type;
    union {
        MiniGameIceSkating ice;
        MiniGameBinary binary;
    };
};

function void ModeMiniGame(Game_State *state);
function void UpdateMGPlayer(MiniGamePlayer *player, Input *input, MiniGameIceSkating *minigame, Game_State *state);
function void BuildMap(Game_State *state);
function void UpdateRenderModeMiniGame(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer);
function void UpdateRenderIceSkating(Game_State *state, Input *input, Draw_Batch *batch);
function void UpdateRenderBinaryCount(Game_State *state, Input *input, Draw_Batch *batch);
#endif  // LUDUM_MODE_MINIGAME_H_
