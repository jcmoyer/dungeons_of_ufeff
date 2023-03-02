#pragma once

#include <string>
#include <cstdint>

struct session_state;
struct entity;

struct npc_context {
    // engine interaction
    virtual void say(const std::string& message) = 0;
    virtual void play_sound(const char* name) = 0;
    virtual void schedule(uint32_t id, double seconds_from_now) = 0;

    // world interaction
    virtual entity* self() const = 0;
    virtual entity* get_entity(const char* name) = 0;
    virtual void set_encounter_state(bool enabled) = 0;

    // quest management
    virtual void set_flag(const char* name, int value) = 0;
    virtual int get_flag(const char* name) = 0;

    // session interaction
    virtual session_state* session() = 0;
};

using npc_interact_script = void(*)(npc_context& context);

npc_interact_script get_npc_interact_script(uint32_t id);
