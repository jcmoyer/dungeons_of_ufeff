#include "encounters.hpp"

#include "../random.hpp"

// clang-format off
encounter_set enc_dungeon{
    {
        {BCI_SLIME, BCI_SLIME, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
        {BCI_SLIME, BCI_SLIME, BCI_SLIME, BCI_SLIME, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
        {BCI_SKELETON, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
        {BCI_SKELETON, BCI_SKELETON, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
        {BCI_BAT, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
        {BCI_SKELETON, BCI_SLIME, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
    }
};
encounter_set enc_sanctum{
    {
        {BCI_GHOST, BCI_SLIME, BCI_GHOST, BCI_SLIME, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
        {BCI_SKELETON, BCI_SKELETON, BCI_SKELETON, BCI_SKELETON, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
        {BCI_SLIME, BCI_SLIME, BCI_SLIME, BCI_SLIME, BCI_SLIME, BCI_SLIME, BCI_SLIME, BCI_SLIME},
        {BCI_SKELETON, BCI_SKELETON, BCI_GHOST, BCI_GHOST, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
    }
};
encounter_set enc_lair{
    {
        {BCI_GHOST, BCI_GHOST, BCI_GHOST, BCI_GHOST, BCI_GHOST, BCI_NULL, BCI_NULL, BCI_NULL},
        {BCI_GHOST, BCI_GHOST, BCI_GHOST, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
        {BCI_BAT, BCI_GHOST, BCI_GHOST, BCI_BAT, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL},
        //{BCI_RAGWORM, BCI_NULL, BCI_NULL, BCI_NULL , BCI_NULL, BCI_NULL, BCI_NULL , BCI_NULL},
    }
};
// clang-format on

const encounter& encounter_set::random_encounter() const
{
    size_t i = (size_t)random::rand_int(0, (int)encounters.size() - 1);
    return encounters[i];
}

const encounter_set& get_encounter_set(uint32_t id)
{
    switch (id)
    {
    case 0:
        return enc_dungeon;
    case 1:
        return enc_sanctum;
    case 2:
        return enc_lair;
    default:
        assert(false && "invalid encounter set id");
        return enc_dungeon;
    }
}

encounter get_final_boss_encounter()
{
    return {{BCI_RAGWORM, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL, BCI_NULL}};
}