#pragma once

#include <cstdint>
#include "controller.hpp"

enum battle_character_info_id {
    BCI_PLAYER,
    BCI_SLIME,
    BCI_SKELETON,
    BCI_BAT,
    BCI_GHOST,
    BCI_SPIDER,
    BCI_RAGWORM,
    BCI_MAX,
    BCI_NULL = -1     // used as a placeholder in encounter arrays
};

struct battle_character_info {
    int32_t max_life;
    uint32_t sprite_id;
    float speed;
    float jump;
    float friction;
    float thrust;
    const char* hurt_sound;
    uint32_t hurt_particle_sprite_id;

    using controller_factory = std::unique_ptr<bc_controller>(*)();
    controller_factory factory;
    int32_t exp;
};

extern battle_character_info B_CHARINFO[BCI_MAX];