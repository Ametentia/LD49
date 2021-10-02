#if !defined(LUDUM_MODE_PLAY_H_)
#define LUDUM_MODE_PLAY_H_

// @Note: I just chose some values here you can look in UpdatePlayer to see how these values are used and
// tweak them to see if there is a better "feel" for the movement
//

// In "world units"
//
#define PLAYER_MAX_JUMP_HEIGHT (1.2f)
#define PLAYER_MIN_JUMP_HEIGHT (0.3f)

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

    v2 p;
    v2 dp;

    Bird_Follower birds[3];

    f32 x_scale;
    v2 dim;
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
};

function void ModePlay(Game_State *state, Random random);
function void UpdateRenderModePlay(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer);

function void UpdatePlayer(Player *player, Input *input);

#endif  // LUDUM_MODE_PLAY_H_
