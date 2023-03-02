#pragma once

#include <SDL.h>

#include "gamestate.hpp"
#include "timer.hpp"

class game;

class st_mainmenu : public gamestate
{
public:
    st_mainmenu(game* owner, shared_state* state);

    void init() override;
    void update() override;
    void render(double a) override;
    void handle_event(const SDL_Event& ev) override;
    void enter(gamestate* old) override;
    void leave() override;

private:
    void begin_game_transition();
    void render_game_fade();

private:
    enum substate
    {
        none,
        fade_to_game,
    };

    substate sub = none;

    timer fade_timer;

    game* owner;
    shared_state* state;
};