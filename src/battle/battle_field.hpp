#pragma once

#include <vector>
#include "battle_character.hpp"
#include "battle_projectile.hpp"
#include "battle_particle.hpp"
#include "battle_fx.hpp"
#include <fmt/format.h>
#include "../random_vec.hpp"
#include "../global_services.hpp"
#include <glm/vec3.hpp>

struct battle_field_bounds {
    float left, right, floor;
};

struct battle_field_properties {
    const char* name;
    battle_field_bounds bounds;
    glm::vec3 light_direction;
    glm::vec3 sky_color;
};

constexpr battle_field_properties BF_DUNGEON = { "bf_dungeon", {7.783096f, 475.339f, 8.890106f }, {0.5, -0.3, -0.8}, {0x14 / 255.f, 0x0c / 255.f, 0x1c / 255.f } };
constexpr battle_field_properties BF_SANCTUM = { "bf_sanctum", {7.783096f, 475.339f, 8.890106f }, {0.2, -0.8, -0.2}, {0x59 / 255.f, 0x7d / 255.f, 0xce / 255.f } };
constexpr battle_field_properties BF_LAIR    = { "bf_lair",    {7.783096f, 475.339f, 8.890106f }, {0.2, -0.8, -0.2}, {0xd0 / 255.f, 0x46 / 255.f, 0x48 / 255.f } };

struct battle_field {
    std::vector<battle_character> characters;
    std::vector<battle_projectile> projectiles;
    battle_particle_system particle_sys;
    battle_fx_system fx_sys;
    battle_field_bounds bounds;

    size_t player_index = -1;
    size_t last_enemy_hit = -1;

    glm::vec2 clamp_to_bounds(glm::vec2 v) {
        v.x = clamp(v.x, bounds.left, bounds.right);
        if (v.y < bounds.floor) {
            v.y = bounds.floor;
        }
        return v;
    }

    void spawn_character(battle_character_info_id id, const glm::vec2& pos) {
        auto& chara = characters.emplace_back(characters.size(), &B_CHARINFO[id]);
        if (chara.info->factory) {
            chara.controller = chara.info->factory();
        }
        chara.pos = chara.prev_pos = pos;
    }

    battle_projectile& spawn_projectile(uint32_t sprite_id) {
        battle_projectile& proj = projectiles.emplace_back(get_animation_set(sprite_id));
        return proj;
    }

    void clear() {
        characters.clear();
        projectiles.clear();
        particle_sys.particles.clear();
        fx_sys.effects.clear();
        player_index = -1;
        bounds = {};
        last_enemy_hit = -1;
    }

    void init_positions() {
        for (size_t i = 0; i < characters.size(); ++i) {
            characters[i].init_position(bounds.floor);
        }
    }

    void update(bool skip_think) {
        for (size_t i = 0; i < characters.size(); ++i) {
            battle_character& char1 = characters[i];

            if (!skip_think && char1.alive && char1.controller) {
                char1.controller->think(char1, *this);
            }

            char1.update(bounds.floor);
            char1.pos.x = clamp(char1.pos.x, bounds.left, bounds.right);

            // skip collision checks for dead characters
            if (!char1.alive) {
                continue;
            }

            if (i == player_index) {
                continue;
            } else {
                if (player().hitstun_frames) {
                    continue;
                }

                rectangle subrect;

                rectangle player_hitbox = player().worldspace_hitbox();

                if (char1.worldspace_hitbox().intersect(player().worldspace_hitbox(), subrect)) {
                    std::string hurt_sound = fmt::format("assets/sound/{}.ogg", player().info->hurt_sound);
                    g_audio->play_sound(hurt_sound.c_str());

                    player().hitstun_frames += 60;
                    //player().vel += glm::vec2{ 2 * char1.vel.x, 0 } +glm::vec2{ 0, 3 };
                    player().vel += glm::vec2{ char1.vel.x, 0 } + glm::vec2{ 0, 3 };
                    player().hurt(1);

                    for (int i = 0; i < 10; ++i)
                        particle_sys.emit(player().info->hurt_particle_sprite_id, { player_hitbox.x + player_hitbox.w / 2, player_hitbox.y + player_hitbox.h / 2 }, char1.vel * 0.25f + rand_vec2(-1, 1, -1, 1), 0.2f).acc = { 0, -0.2f };
                }
            }



            //for (size_t j = i + 1; j < characters.size(); ++j) {
            //    battle_character& char2 = characters[j];

            //    rectangle subrect;
            //    if (char1.worldspace_hitbox().intersect(char2.worldspace_hitbox(), subrect)) {

            //    }
            //}

        }

        //for (auto& c : characters) {
        //    if (c.controller) {
        //        c.controller->think(c, *this);
        //    }

        //    c.update(bounds.floor);
        //    c.pos.x = clamp(c.pos.x, bounds.left, bounds.right);
        //}

        for (auto& p : projectiles) {
            p.update(*this);

            rectangle proj_rect = p.worldspace_hitbox();
            rectangle unused;

            assert(p.owner != -1);
            bool is_monster_owned = p.owner != player_index;

            for (size_t i = 0; i < characters.size(); ++i) {
                if (i == player_index && !is_monster_owned) {
                    continue;
                } else if (i != player_index && is_monster_owned) {
                    continue;
                }

                battle_character& c = characters[i];
                rectangle char_rect = c.worldspace_hitbox();

                if (c.hitstun_frames) {
                    continue;
                }

                if (proj_rect.intersect(char_rect, unused)) {
                    std::string hurt_sound = fmt::format("assets/sound/{}.ogg", c.info->hurt_sound);
                    g_audio->play_sound(hurt_sound.c_str());

                    for (int i = 0; i < 10; ++i)
                        particle_sys.emit(c.info->hurt_particle_sprite_id, { char_rect.x + char_rect.w / 2, char_rect.y + char_rect.h / 2 }, -p.vel * 0.25f + rand_vec2(-1, 1, -1, 1), 0.2f).acc = { 0, -0.2f };

                    if (proj_rect.x <= char_rect.x) {
                        fx_sys.spawn(17, right, { -12, 0 }).attach(i);
                    } else {
                        fx_sys.spawn(17, left, { 12, 0 }).attach(i);
                    }

                    if (i == player_index) {
                        c.hitstun_frames += 60;
                        c.vel += p.vel + glm::vec2{ 0, 1 };
                    } else {
                        c.hitstun_frames += 1;
                        //c.vel += p.vel + (glm::vec2{ 0, 1 } *(float)c.hitstun_frames); // TODO: cute scaling idea but maybe too over the top :V)
                        c.vel += glm::normalize(p.vel) + (glm::vec2{ 0, 1 } *(float)c.hitstun_frames); // TODO: cute scaling idea but maybe too over the top :V)
                        last_enemy_hit = i;
                    }

                    if (p.pierce) {
                        --p.pierce;
                    } else {
                        p.kill();
                    }

                    c.hurt(p.power);
                }

                if (!p.alive) {
                    break;
                }
            }

            if (p.pos.x < bounds.left - 200.f || p.pos.x > bounds.right + 200.f || p.pos.y < bounds.floor - 200.f) {
                p.kill();
            }
        }

        if (projectiles.size()) {
            projectiles.erase(
                std::remove_if(projectiles.begin(), projectiles.end(), [](const battle_projectile& p) {
                    return !p.alive;
                    }), projectiles.end());
        }

        particle_sys.update(bounds.floor);
        fx_sys.update();
    }

    battle_character& player() {
        assert(player_index != -1);
        return characters[player_index];
    }

    void set_player_index(size_t i) {
        assert(i < characters.size());
        player_index = i;
    }

    bool hostiles_alive() const {
        for (size_t i = 0; i < characters.size(); ++i) {
            if (i == player_index) continue;
            if (characters[i].alive) {
                return true;
            }
        }
        return false;
    }

    int32_t calc_exp_value() const {
        int32_t sum = 0;
        for (const auto& ch : characters) {
            sum += ch.info->exp;
        }
        return sum;
    }
};