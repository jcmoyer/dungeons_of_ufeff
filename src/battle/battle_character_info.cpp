#include "battle_character_info.hpp"

// clang-format off
battle_character_info B_CHARINFO[BCI_MAX]{
    {5,  0, 4.f,  4.f, 0.65f, 0.50f, "stab",   16, nullptr, 0},
    {5,  4, 2.f,  5.f, 0.98f, 0.10f, "slime0", 21, slime_controller::create, 1},
    {15, 3, 2.5f, 3.f, 0.65f, 0.70f, "bone",   22, skeleton_controller::create, 3},
    {15, 5, 3.0f, 0.f, 0.98f, 1.00f, "stab",   16, bat_controller::create, 5},
    {30, 6, 2.5f, 3.f, 0.65f, 0.70f, "stab",   23, ghost_controller::create, 10},
    // spider
    {15, 7, 2.5f, 3.f, 0.65f, 0.70f, "stab",   16, skeleton_controller::create, 10},
    // THAT guy
    {1000,  0, 4.f,  4.f, 0.65f, 0.50f, "stab",   16, ragworm_controller::create, 1000},
};
