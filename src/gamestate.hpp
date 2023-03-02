#pragma once

#include <cstdint>
#include <unordered_map>
#include <SDL.h>

class texture_manager;
class spritebatch;
struct world;
class quad_renderer;
class bmfont;
class audio_system;

struct input_state {
    int mouse_x = 0;
    int mouse_y = 0;
    bool left_mouse = false;
    std::unordered_map<uint32_t, bool> keys;
};

struct player_stats {
    int32_t level = 1;
    int32_t player_exp = 0;

    int32_t max_life() const {
        return 5 + (level - 1) * 3;
    }

    int32_t exp_tnl() const {
        return level * 5;
    }

    int32_t power() const {
        return (level + 5) / 5;
    }

    bool can_levelup() const {
        return player_exp >= exp_tnl();
    }

    void levelup() {
        player_exp -= exp_tnl();
        ++level;
    }

    void add_exp(int32_t amt) {
        player_exp += amt;
        while (can_levelup()) {
            levelup();
        }
    }

    bool operator==(player_stats& rhs) const {
        return level == rhs.level && player_exp == rhs.player_exp;
    }

    bool operator!=(player_stats& rhs) const {
        return !(*this == rhs);
    }
};

// contains state related to the current game session: non-system state such as player unlocks
struct session_state {
    bool has_flashjump = false;
    bool has_avenger = false;
    bool has_shadowpartner = false;
    player_stats stats;
};

struct shared_state {
    texture_manager* texman;
    spritebatch* batch;
    uint32_t frame_counter;
    input_state* input;
    quad_renderer* quad_render;
    bmfont* font;
    audio_system* audio;
    session_state* session;
};

class gamestate {
public:
    virtual void enter() = 0;
    virtual void leave() = 0;
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void render(double alpha) = 0;
    virtual void handle_event(const SDL_Event& ev) = 0;
};
