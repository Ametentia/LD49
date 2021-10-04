#if !defined(LUDUM_MODE_MENU_H_)
#define LUDUM_MODE_MENU_H_

struct Mode_Menu {
    Sprite_Animation animation;
    Random random;
};

function void ModeMenu(Game_State *state, Random *random);
function void UpdateRenderModeMenu(Game_State *state, Input *input, Renderer_Buffer *renderer_buffer);
#endif  // LUDUM_MODE_MENU_H_
