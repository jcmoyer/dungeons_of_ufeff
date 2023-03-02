#pragma once

#include <SDL.h>
#include <cassert>
#include <cstdint>
#include <string_view>
#include <unordered_map>

class texture_manager;
class spritebatch;
struct world;
class quad_renderer;
class bmfont;
class audio_system;

enum input_action
{
    IA_INTERACT,
    IA_JUMP,
    IA_SKILL1,
    IA_SKILL2,
};

constexpr bool is_battle_action(input_action act)
{
    switch (act)
    {
    case IA_JUMP:
    case IA_SKILL1:
    case IA_SKILL2:
        return true;
    default:
        return false;
    }
}

constexpr std::string_view get_action_name(input_action act)
{
    using namespace std::string_view_literals;
    switch (act)
    {
    case IA_INTERACT:
        return "Interact"sv;
    case IA_JUMP:
        return "Jump"sv;
    case IA_SKILL1:
        return "Skill 1"sv;
    case IA_SKILL2:
        return "Skill 2"sv;
    default:
        assert(false && "bad action value");
        return "Unknown"sv;
    }
}

struct action_map
{
    std::unordered_map<input_action, SDL_Keycode> keybinds;

    action_map()
    {
        keybinds[IA_INTERACT] = SDLK_x;
        keybinds[IA_JUMP] = SDLK_LALT;
        keybinds[IA_SKILL1] = SDLK_x;
        keybinds[IA_SKILL2] = SDLK_c;
    }

    SDL_Keycode get_key(input_action act) const
    {
        return keybinds.at(act);
    }

    void rebind_key(input_action act, SDL_Keycode key)
    {
        if (act == IA_INTERACT)
        {
            keybinds[act] = key;
            return;
        }
        else
        {
            // the other keys are all used in the battle state, so we prevent duplicate binds
            for (auto& [k, v] : keybinds)
            {
                if (v == key && is_battle_action(k))
                {
                    v = keybinds[act];
                    break;
                }
            }
            keybinds[act] = key;
        }
    }
};

struct input_state
{
    int mouse_x = 0;
    int mouse_y = 0;
    bool left_mouse = false;
    std::unordered_map<uint32_t, bool> keys;
};

struct player_stats
{
    int32_t level = 1;
    int32_t player_exp = 0;

    int32_t max_life() const
    {
        return 5 + (level - 1) * 3;
    }

    int32_t exp_tnl() const
    {
        return level * 5;
    }

    int32_t power() const
    {
        return (level + 5) / 5;
    }

    bool can_levelup() const
    {
        return player_exp >= exp_tnl();
    }

    void levelup()
    {
        player_exp -= exp_tnl();
        ++level;
    }

    void add_exp(int32_t amt)
    {
        player_exp += amt;
        while (can_levelup())
        {
            levelup();
        }
    }

    bool operator==(player_stats& rhs) const
    {
        return level == rhs.level && player_exp == rhs.player_exp;
    }

    bool operator!=(player_stats& rhs) const
    {
        return !(*this == rhs);
    }
};

// contains state related to the current game session: non-system state such as player unlocks
struct session_state
{
    bool has_flashjump = false;
    bool has_avenger = false;
    bool has_shadowpartner = false;
    player_stats stats;
    action_map keybinds;
};

struct shared_state
{
    texture_manager* texman;
    spritebatch* batch;
    uint32_t frame_counter;
    input_state* input;
    quad_renderer* quad_render;
    bmfont* font;
    audio_system* audio;
    session_state* session;
};

class gamestate
{
public:
    virtual ~gamestate() {}
    virtual void enter(gamestate* old) = 0;
    virtual void leave() = 0;
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void render(double alpha) = 0;
    virtual void handle_event(const SDL_Event& ev) = 0;
};
