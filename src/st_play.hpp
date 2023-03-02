#pragma once

struct shared_state;
class game;
struct texture;
class water_renderer;
struct foam_emitter;

#include <SDL.h>
#include "camera.hpp"
#include "world.hpp"
#include "dialoguebox.hpp"
#include "gamestate.hpp"
#include "timer.hpp"
#include "npc.hpp"
#include <unordered_map>
#include <deque>
#include <glm/vec2.hpp>
#include "battle/encounters.hpp"

struct audio_parameters;

struct scheduled_script {
    timer t;
    uint32_t script_id;
};

struct battle_transition_particle {
    glm::vec2 pos, prev_pos, vel;
};

// TODO yeah these need to be moved get over it
struct water_draw_cmd {
    rectangle dest;
    float world_x, world_y;
};

struct light_draw_cmd {
    uint32_t world_x, world_y;
    float radius;
};

class st_play : public gamestate {
public:
    st_play(game* owner, shared_state* state);

    void init() override;
    void update() override;
    void render(double a) override;
    void handle_event(const SDL_Event& ev) override;
    void enter() override;
    void leave() override;

private:
    void show_message(std::string_view m);
    void render_map_intro();
    void render_map_fadeout();
    void begin_map_transition(const std::string& map_name, const std::string& exit_name);
    void begin_map_intro();
    void try_move_player(direction d);
    void begin_battle_transition(encounter enc);
    void render_battle_fade(double a);
    void display_next_message();
    void process_scheduled_scripts();
    void emit_battle_transition_particles();
    void render_mlp_fade();
    encounter pick_random_map_encounter();

private:
    game* owner;
    shared_state* state;

    camera cam;
    camera prev_cam;

    world wor;

    const texture* t_atlas;
    const texture* t_water_base;
    const texture* t_water_foam;
    const texture* t_waterfall;
    const texture* t_dialogue_back;
    const texture* t_light_mask;
    const texture* t_lava_base;
    const texture* t_lava_blend;

    std::unique_ptr<water_renderer> water_render;
    std::unique_ptr<foam_emitter> foam_em;

    std::shared_ptr<audio_parameters> current_music;

    size_t player_handle;
    int steps = 0;

    enum substate {
        none,
        message,

        map_intro_title,
        map_intro_subtitle,
        map_intro_fadein,

        // portan to another map
        map_exit_fadeout,

        // portal within a map
        map_local_portal_fadeout,
        map_local_portal_fadein,

        // transitioning to battle
        battle_fadeout,

        // transitioning back from battle
        battle_fadein,
    };

    substate sub = none;

    std::deque<std::string> message_queue;

    dialogue_info m_info;
    std::string m_text;
    rectangle m_rect;

    timer mi_timer;

    timer me_timer;
    world me_new_world;

    timer mlp_fade_timer;
    uint32_t mlp_exit_x;
    uint32_t mlp_exit_y;

    timer b_fade_timer;
    encounter b_next_encounter;

    std::unordered_map<std::string, int> flags;
    std::vector<scheduled_script> scheduled_scripts;

    std::vector<battle_transition_particle> transition_particles;

    bool encounters_enabled = true;

    std::string current_map_name;

    std::vector<water_draw_cmd> water_cmds;
    std::vector<water_draw_cmd> waterfall_cmds;
    std::vector<water_draw_cmd> lava_cmds;
    std::vector<light_draw_cmd> light_cmds;

    friend class st_interact_context;
};

class st_interact_context : public npc_context {
public:
    st_interact_context(st_play* o, entity* e) : owner{ o }, ent{ e } {}

private:
    st_play* owner;
    entity* ent;

    // Inherited via npc_context
    void say(const std::string& message) override;
    entity* self() const override;
    void play_sound(const char* name) override;
    void schedule(uint32_t id, double seconds_from_now) override;
    entity* get_entity(const char* name) override;
    void set_flag(const char* name, int value) override;
    int get_flag(const char* name) override;
    session_state* session() override;
    void set_encounter_state(bool enabled) override;
};