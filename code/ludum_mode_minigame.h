#if !defined(LUDUM_MODE_MINIGAME_H_)
#define LUDUM_MODE_MINIGAME_H_

enum MiniGameType {
    MiniGame_BinaryCount = 0,
    MiniGame_IceSkating
};

struct MiniGamePlayer{
    v2 pos;
    bool alive;
    u32 xOnGrid;
    u32 yOnGrid;
    v2 dim;
    f32 rotation;
    v2 scale;
    Sprite_Animation animation;
};

enum MiniGameTileType{
    Nothing = -1,
    Blocked = 0,
    Open = 1
};

struct MiniGameTile{
    MiniGameTileType type;
    Image_Handle asset;
    v2 p;
    v2 dim;
    bool walkedOn;
};

struct MiniGameIceSkating {
    bool won;
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
