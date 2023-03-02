#pragma once

#include <SDL.h>
#include <glm/vec2.hpp>
#include <memory>

#include "audio.hpp"
#include "bmfont.hpp"
#include "camera.hpp"
#include "foam_emitter.hpp"
#include "gamestate.hpp"
#include "imm_renderer.hpp"
#include "quad_renderer.hpp"
#include "screen_renderer.hpp"
#include "shader.hpp"
#include "spritebatch.hpp"
#include "st_battle.hpp"
#include "st_battlestats.hpp"
#include "st_gameover.hpp"
#include "st_gamewin.hpp"
#include "st_mainmenu.hpp"
#include "st_options.hpp"
#include "st_play.hpp"
#include "texture_manager.hpp"
#include "timer.hpp"
#include "water_renderer.hpp"
#include "world.hpp"

enum class transition_to
{
    mainmenu,
    play,
    battle,
    gameover,
    battlestats,
    gamewin,
    options,
};

const int INTERNAL_WIDTH = 512;
const int INTERNAL_HEIGHT = 256;

class game
{
public:
    game();
    ~game();

    void run();

    void transition(transition_to t);
    void transition(gamestate* t);

    timer create_timer(double duration_sec);

    void render_to_scene();
    void render_to_mask();
    void clear_mask();
    void set_mask_effect(float amt);
    void apply_mask();

    st_battle& get_battle_state();
    st_battlestats& get_battlestats_state();
    st_options& get_options_state();

    int get_scale() const;
    glm::ivec2 unproject(const glm::ivec2& vec);

    void select_next_resolution();
    void toggle_fullscreen();

private:
    void handle_event(const SDL_Event& ev);

    void init();
    void update();
    void render(double a);

    void perform_layout();

private:
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;

    bool running = false;

    camera prev_cam;
    camera cam;

    texture_manager texman;
    bmfont font;

    input_state input;
    session_state session;

    std::unique_ptr<imm_renderer> immr;
    std::unique_ptr<screen_renderer> screen_render;

    audio_system audio;

    GLuint scene_framebuf;
    GLuint scene_color;
    GLuint scene_color_extra;
    GLuint scene_renderbuf;

    GLuint mask_framebuf;
    GLuint mask_color;
    GLuint mask_renderbuf;
    float mask_effect = 0;

    const texture* t_mask;

    uint32_t frame_counter = 0;

    world wor;

    std::unique_ptr<spritebatch> batch;
    std::unique_ptr<quad_renderer> quad_render;

    std::unique_ptr<st_play> play;
    std::unique_ptr<st_mainmenu> mainmenu;
    std::unique_ptr<st_battle> battle;
    std::unique_ptr<st_gameover> gameover;
    std::unique_ptr<st_battlestats> battlestats;
    std::unique_ptr<st_gamewin> gamewin;
    std::unique_ptr<st_options> options;
    gamestate* current_st = nullptr;

    shared_state sstate;

    size_t scales_index = 1;
};
