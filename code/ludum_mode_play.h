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
#define PLAYER_MAX_DOUBLE_JUMP_HEIGHT (1.05f)
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
#define CAMERA_STIFFNESS (28.0f)
#define CAMERA_DAMPING   (12.0f)

enum Player_Flags {
    Player_OnGround   = (1 << 0),
    Player_Drilling   = (1 << 1),
    Player_DoubleJump = (1 << 2),
    Player_Attacking  = (1 << 3)
};

enum Player_Animation {
    PlayerAnimation_Idle = 0,
    PlayerAnimation_Run,

    PlayerAnimation_Count
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

    Sprite_Animation attack_anim;
    Sprite_Animation drill_anim;

    // These are specifically for player body
    //
    Sprite_Animation *animations;
    Player_Animation cur_anim;

    Bird_Follower birds[3];

    f32 facing;
    v2 dim;
    v2 visual_dim;
    v2 visual_offset;

    v2 drill_p;
    v2 drill_dp;

    f32 drill_particle_time;
    Playing_Sound *drill_hit_sound;
};

struct Enemy {
    b32 alive;

    Sprite_Animation anim;

    v2 p;
    v2 dp;

    f32 x_scale;
    f32 decision_wait;
};

enum Tile_Type {
    Tile_Air = -3,
    Tile_Exit,
    Tile_Entrance,
    Tile_Ground,
};

struct Tile {
    Image_Handle image;

    Tile_Type type;
    v2u grid_p;
    f32 drill_time;

    v2 scale;
};

struct Mode_MiniGame;

struct Particle {
    Image_Handle image;

    v3 p;
    v3 dp;

    f32 a;
    f32 da;

    f32 s;
    f32 ds;

    f32 t;
};

struct Mode_Play {
    Memory_Arena *arena;

    Playing_Sound *music;

    // Player
    //
    Player player;
    Random random;
    u32 lives;

    u32 count;
    u32 next_last_p;
    v2 last_p[128];

    u32 enemy_count;
    Enemy enemies[128];

    // Camera movement
    //
    v2 camera_p;
    v2 camera_dp;

    v2 shake_offset;
    f32 shake_angle;

    f32 shake_t;
    f32 shake;

    b32 debug_camera_enabled;
    v3 debug_camera_p;

    // World data
    //
    Tile *tiles;
    Mode_MiniGame *minigame;

    u32 max_particles;
    u32 next_particle;
    Particle *particle_cache;
};

function void ModePlay(Game_State *state, Random random);
function void UpdateRenderModePlay(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer);

function void UpdatePlayer(Mode_Play *play, Player *player, Input *input, Game_State *state);

function b32 IsValidTile(v2s tile_p); // Checks if the tile position is valid within the world grid
function u32 GetCloseTiles(v2 p, Tile *tiles, Tile **out); // Get all of the tiles close to the position

function void SpawnDrillDebris(Mode_Play *play, Asset_Manager *assets, v2 p);

function void UpdateRenderParticles(Draw_Batch *batch, Mode_Play *play, f32 dt);

function void UpdateEnemy(Game_State *state, Mode_Play *play, Enemy *enemy, f32 dt);

#endif  // LUDUM_MODE_PLAY_H_
