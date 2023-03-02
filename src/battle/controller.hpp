#pragma once

#include <cstdint>
#include <glm/vec2.hpp>
#include <memory>

#include "../direction.hpp"
#include "../random.hpp"
#include "skill.hpp"

struct battle_character;
struct battle_field;

struct bc_controller
{
    virtual ~bc_controller() {}
    virtual void think(battle_character& self, battle_field& field) = 0;
};

struct slime_controller : bc_controller
{
    uint32_t jump_timer;

    void think(battle_character& self, battle_field& field);

    slime_controller()
    {
        jump_timer = random::rand_int(100, 200);
    }

    static std::unique_ptr<bc_controller> create()
    {
        return std::make_unique<slime_controller>();
    }
};

struct skeleton_controller : bc_controller
{
    void think(battle_character& self, battle_field& field);

    static std::unique_ptr<bc_controller> create()
    {
        return std::make_unique<skeleton_controller>();
    }
};

struct bat_controller : bc_controller
{
    uint32_t state_duration = 0;
    glm::vec2 target{};

    enum state
    {
        chase,
        hover
    };

    state st = chase;

    void think(battle_character& self, battle_field& field);

    static std::unique_ptr<bc_controller> create()
    {
        return std::make_unique<bat_controller>();
    }
};

struct ghost_controller : bc_controller
{
    uint32_t state_duration = 90;
    glm::vec2 target{};

    enum state
    {
        rush,
        idle
    };

    state st = idle;

    void think(battle_character& self, battle_field& field);

    static std::unique_ptr<bc_controller> create()
    {
        return std::make_unique<ghost_controller>();
    }
};

struct ragworm_controller : bc_controller
{
    ragworm_meteor sk_meteor;
    ragworm_burst sk_burst;
    ragworm_teleport sk_teleport;

    enum state
    {
        none,
        walk_towards_player,
        exhausted,
    };
    state st = none;
    uint32_t frames_walking = 0;
    uint32_t exhaust_timer = 0;
    glm::vec2 walk_target{};
    int last_choice = -1;
    bool could_act_last_frame = true;

    void think(battle_character& self, battle_field& field);

    static std::unique_ptr<bc_controller> create()
    {
        return std::make_unique<ragworm_controller>();
    }
};