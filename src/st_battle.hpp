#pragma once

#include <SDL.h>

#include "battle/battle_camera.hpp"
#include "battle/battle_field.hpp"
#include "battle/encounters.hpp"
#include "battle_field_renderer.hpp"
#include "battle_object_renderer.hpp"
#include "gamestate.hpp"
#include "timer.hpp"

struct audio_parameters;
class game;

struct cached_mesh
{
    battle_field_mesh mesh;
    battle_field_bounds bounds;
};

class st_battle : public gamestate
{
public:
    st_battle(game* owner, shared_state* state);

    // gamestate
    void init() override;
    void update() override;
    void render(double a) override;
    void handle_event(const SDL_Event& ev) override;
    void enter(gamestate* old) override;
    void leave() override;

    void set_battle_field_name(std::string bf_name);
    void set_encounter(encounter enc_);

private:
    void begin_transition();
    void render_fade();
    cached_mesh& get_mesh(const char* name);
    void render_health_bar();

private:
    game* owner;
    shared_state* state;

    battle_object_renderer bo_render;
    battle_field_renderer bf_render;
    const battle_field_properties* bf_props;

    std::shared_ptr<audio_parameters> current_music;

    enum substate
    {
        none,
        battle_fadein,
        battle_fadeout
    };

    enum battle_result
    {
        win,
        lose
    };

    substate sub = none;
    timer b_fade_timer;

    std::unordered_map<std::string, cached_mesh> meshes;

    battle_camera b_cam;
    battle_field b_field;

    std::string bf_name;
    encounter enc;

    battle_result b_result;
};