#pragma once

#include <SDL.h>

#include "gamestate.hpp"
#include "timer.hpp"

class game;

class st_gamewin : public gamestate
{
public:
    st_gamewin(game* owner, shared_state* state);

    void init() override;
    void update() override;
    void render(double a) override;
    void handle_event(const SDL_Event& ev) override;
    void enter(gamestate* old) override;
    void leave() override;

private:
    void render_fade();

private:
    enum substate
    {
        none,
        fade_in,
    };

    substate sub = none;

    timer fade_timer;

    game* owner;
    shared_state* state;
};