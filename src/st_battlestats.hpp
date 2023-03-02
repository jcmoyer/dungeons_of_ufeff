#pragma once

#include <SDL.h>
#include "gamestate.hpp"
#include "timer.hpp"

class game;

class st_battlestats : public gamestate {
public:
    st_battlestats(game* owner, shared_state* state);

    void init() override;
    void update() override;
    void render(double a) override;
    void handle_event(const SDL_Event& ev) override;
    void enter() override;
    void leave() override;

    void set_exp_gained(int32_t amt);

private:
    void render_fade();

private:
    enum substate {
        none,
        fade_in,
        tally,

        fade_out,
    };

    substate sub = none;

    timer fade_timer;

    game* owner;
    shared_state* state;

    int32_t exp_gained;

    player_stats old_stats_snapshot;
    player_stats tally_stats;
    player_stats last_tally_stats;
};