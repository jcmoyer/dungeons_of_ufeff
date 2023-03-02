#pragma once

#include "battle_character_info.hpp"
#include <vector>
#include <algorithm>

struct encounter {
    battle_character_info_id enemies[8];

    bool operator==(const encounter& rhs) const {
        return std::equal(std::cbegin(enemies), std::cend(enemies), std::cbegin(rhs.enemies), std::cend(rhs.enemies));
    }
};

struct encounter_set {
    std::vector<encounter> encounters;

    const encounter& random_encounter() const;
};

const encounter_set& get_encounter_set(uint32_t id);
encounter get_final_boss_encounter();