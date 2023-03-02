#pragma once

#include <SDL.h>

#include "gamestate.hpp"
#include "rectangle.hpp"

class game;

struct basic_button
{
    rectangle rect;
    std::string text;
};

struct option_button : public basic_button
{
    input_action act;

    option_button(input_action a)
        : act{a} {}

    void update_text(const action_map& m);
};

struct resolution_button : public basic_button
{
    void update_text(int scale);
};

struct toggle_fullscreen_button : public basic_button
{
    toggle_fullscreen_button()
    {
        text = "Toggle Fullscreen";
    }
};

class st_options : public gamestate
{
public:
    st_options(game* owner, shared_state* state);

    void init() override;
    void update() override;
    void render(double a) override;
    void handle_event(const SDL_Event& ev) override;
    void enter(gamestate* old) override;
    void leave() override;

private:
    game* owner;
    shared_state* state;

    enum substate
    {
        waiting_for_key,
        none,
    };

    substate sub = none;

    std::vector<option_button> input_buttons;
    resolution_button res_button;
    toggle_fullscreen_button fs_button;

    option_button* pending_button = nullptr;
    gamestate* previous_state = nullptr;
};
