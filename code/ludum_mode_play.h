#if !defined(LUDUM_MODE_PLAY_H_)
#define LUDUM_MODE_PLAY_H_

// @Note: I just chose some values here you can look in UpdatePlayer to see how these values are used and
// tweak them to see if there is a better "feel" for the movement
//

// In elementary units
//
#define WORLD_X_SIZE (100)
#define WORLD_Y_SIZE (100)

// In "world units"
//
#define PLAYER_MAX_JUMP_HEIGHT (1.2f)
#define PLAYER_MIN_JUMP_HEIGHT (0.3f)
#define WORLD_TILE_SIZE (0.25f)

// In seconds
//
#define PLAYER_JUMP_APEX_TIME   (0.5f)
#define PLAYER_JUMP_BUFFER_TIME (0.2f)
#define PLAYER_COYOTE_TIME      (0.2f)
#define TILE_DRILL_TIME         (0.75f)

// Slow down due to "friction"
//
#define PLAYER_DAMPING (18.5f)

// Max speed in each direction. This technically means you can go "faster" diagonally
//
#define PLAYER_MAX_SPEED_X (2.2f)
#define PLAYER_MAX_SPEED_Y (4.3f)
#define PLAYER_MAX_SPEED_X_DRILLING (1.1f)

// Movement
//
#define PLAYER_MOVE_SPEED (10)
#define PLAYER_AIR_STRAFE_SPEED (4)

// Camera movement
//
#define CAMERA_STIFFNESS (20.0f)
#define CAMERA_DAMPING   (12.0f)

enum Player_Flags {
    Player_OnGround = (1 << 0),
    Player_Drilling = (1 << 1)
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
    f32 last_on_ground_time;

    v2 p;
    v2 dp;
    Sprite_Animation animations[2];
    Player_Animation current_animation;
    Bird_Follower birds[3];

    f32 x_scale;
    v2 dim;
    v2 visual_dim;
    v2 visual_offset;
    Playing_Sound drill_hit_sound;
    u8 drill_hit_playing;
};

enum Tile_Type {
    Tile_Air = 0,
    Tile_Ground
};

struct Tile {
    Tile_Type type;
    v2u grid_p;
    f32 drill_time;
};
struct Mode_MiniGame;
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

    b32 debug_camera_enabled;
    v3 debug_camera_p;

    // World data
    //
    Tile *tiles;
    Mode_MiniGame *minigame;
};

function void ModePlay(Game_State *state, Random random);
function void UpdateRenderModePlay(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer);

function void UpdatePlayer(Mode_Play *play, Player *player, Input *input, Game_State *state);

#endif  // LUDUM_MODE_PLAY_H_
