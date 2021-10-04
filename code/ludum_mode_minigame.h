struct MiniGamePlayer{
    f32 x_scale;
    v2 pos;
    v2 dim;
    Sprite_Animation animation;
};

struct MiniGameTile{
    s32 value;
    Image_Handle asset;
    v2 p;
    v2 dim;
    bool walkedOn;
};

struct Mode_MiniGame{
    MiniGamePlayer player;
    MiniGameTile tiles[12];
    u32 widths[4];
    u32 heights[4];
    u32 width;
    u32 height;
};

function void ModeMiniGame(Game_State *state);
function void UpdateMGPlayer(MiniGamePlayer *player, Input *input, Mode_MiniGame *minigame, Game_State *state);
function void BuildMap(Game_State *state);