#if !defined(LUDUM_MODE_PLAY_H_)
#define LUDUM_MODE_PLAY_H_

// @Note: I just chose some values here you can look in UpdatePlayer to see how these values are used and
// tweak them to see if there is a better "feel" for the movement
//

// In elementary units
//
#define WORLD_X_SIZE (50)
#define WORLD_Y_SIZE (50)

// In "world units"
//
#define PLAYER_MAX_JUMP_HEIGHT (1.2f)
#define PLAYER_MIN_JUMP_HEIGHT (0.3f)
#define WORLD_TILE_SIZE (0.25f)

// In seconds
//
#define PLAYER_JUMP_APEX_TIME   (0.5f)
#define PLAYER_JUMP_BUFFER_TIME (0.2f)
#define PLAYER_COYOTE_TIME      (0.1f)

// Slow down due to "friction"
//
#define PLAYER_DAMPING (18.5f)

// Max speed in each direction. This technically means you can go "faster" diagonally
//
#define PLAYER_MAX_SPEED_X (2.2f)
#define PLAYER_MAX_SPEED_Y (4.3f)

// Movement
//
#define PLAYER_MOVE_SPEED (10)
#define PLAYER_AIR_STRAFE_SPEED (4)

// Camera movement
//
#define CAMERA_STIFFNESS (20.0f)
#define CAMERA_DAMPING   (12.0f)

enum Player_Flags {
    Player_OnGround = (1 << 0)
};

enum Player_Animation {
    Player_Idle = 0,
    Player_Run
};

struct Bird_Follower {
    b32 alive;

    Image_Handle image;

    v2 p;
    v2 dp;

    f32 cp, vp;
    v2 offset;

    f32 dir_timer;
    f32 x_scale;
};

struct Player {
    u32 flags;
    f32 last_jump_time;
    f32 time_off_ground;

    v2 p;
    v2 dp;
    Sprite_Animation animations[2];
    Player_Animation current_animation;
    Bird_Follower birds[3];

    f32 x_scale;
    v2 dim;
};

struct Tile {
    v2 p;
    v2 dim;
    Image_Handle asset;
    u8 alive;
};

struct Mode_Play {
    Memory_Arena *arena;

    // Player
    //
    Player player;

    Random random;

    u32 count;
    u32 next_last_p;
    v2 last_p[128];

    // Camera movement
    //
    v2 camera_p;
    v2 camera_dp;

    // World data
    //
    Tile tiles[WORLD_X_SIZE][WORLD_Y_SIZE];
};

function void ModePlay(Game_State *state, Random random);
function void UpdateRenderModePlay(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer);

function void UpdatePlayer(Player *player, Input *input, Mode_Play *play, Draw_Batch *draw_batch);

#endif  // LUDUM_MODE_PLAY_H_
