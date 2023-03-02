#pragma once

#include <SDL.h>
#include "camera.hpp"
#include "texture_manager.hpp"
#include "bmfont.hpp"
#include "imm_renderer.hpp"
#include <memory>
#include "audio.hpp"
#include "spritebatch.hpp"
#include "quad_renderer.hpp"
#include "water_renderer.hpp"
#include "foam_emitter.hpp"

#include "shader.hpp"
#include "world.hpp"

#include "st_play.hpp"
#include "st_mainmenu.hpp"
#include "st_battle.hpp"
#include "st_gameover.hpp"
#include "st_battlestats.hpp"
#include "st_gamewin.hpp"
#include "gamestate.hpp"
#include "timer.hpp"
#include "screen_renderer.hpp"

enum class transition_to {
    mainmenu,
    play,
    battle,
    gameover,
    battlestats,
    gamewin,
};

const int INTERNAL_WIDTH = 512;
const int INTERNAL_HEIGHT = 256;

class game {
public:
    game();
    ~game();

    void run();

    void transition(transition_to t);
    timer create_timer(double duration_sec);

    void render_to_scene();
    void render_to_mask();
    void clear_mask();
    void set_mask_effect(float amt);
    void apply_mask();

    st_battle& get_battle_state();
    st_battlestats& get_battlestats_state();

private:
    void handle_event(const SDL_Event& ev);

    void init();
    void update();
    void render(double a);

    void perform_layout();
    void select_next_resolution();
    void toggle_fullscreen();

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

    bool render_solid = true;

    std::unique_ptr<st_play> play;
    std::unique_ptr<st_mainmenu> mainmenu;
    std::unique_ptr<st_battle> battle;
    std::unique_ptr<st_gameover> gameover;
    std::unique_ptr<st_battlestats> battlestats;
    std::unique_ptr<st_gamewin> gamewin;
    gamestate* current_st = nullptr;

    shared_state sstate;
};
