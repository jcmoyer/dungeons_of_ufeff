#pragma once

#include <glm/vec2.hpp>
#include <memory>
#include "skill.hpp"
#include "../animation.hpp"
#include "../tilemap.hpp"
#include "battle_character_info.hpp"
#include "../mathutil.hpp"
#include "../animation_data.hpp"
#include <glm/gtx/compatibility.hpp>

struct skill;

struct battle_character {
    size_t id = -1;
    direction facing = down;
    glm::vec2 pos{};
    glm::vec2 prev_pos{};
    glm::vec2 vel{};
    bool grounded = true;
    uint32_t frames_since_jump = 0;
    animator anim;
    uint32_t hitstun_frames = 0;
    int32_t life;
    bool alive = true;
    bool flying = false;
    int32_t power = 1;
    
    uint32_t frames_since_skill_start = 0;
    skill* current_skill = nullptr;
    const battle_character_info* info;
    std::unique_ptr<bc_controller> controller;

    battle_character(size_t id_, const battle_character_info* i) {
        id = id_;
        info = i;
        anim.set_animation_set(get_animation_set(i->sprite_id));
        anim.set_animation("down");

        life = i->max_life;
    }

    void use_skill(battle_field& b_field, skill* sk);
    void use_skill_unconditionally(battle_field& b_field, skill* sk);

    void set_fly_state(bool is_flying) {
        flying = is_flying;
    }

    void kill() {
        alive = false;
        anim.set_animation("dead");
    }

    void hurt(int32_t amt) {
        life -= amt;
        if (life <= 0) {
            kill();
            life = 0;
        }
    }

    void update(float floor);

    bool can_act() const {
        return alive && current_skill == nullptr;
    }

    void jump() {
        if (grounded) {
            grounded = false;
            vel.y = info->jump;
        }
    }

    void move_left() {
        if (!alive) {
            return;
        }

        const float SPEED = info->speed;

        if (grounded) {
            vel.x -= SPEED * info->thrust;
            vel.x = clamp(vel.x, -SPEED, SPEED);
        } else {
            // soft clamp
            const float amount = SPEED * 0.01f;
            if (vel.x - amount >= -SPEED) {
                vel.x -= amount;
            }
        }
        facing = left;
        anim.set_animation("left");
    }

    void move_right() {
        if (!alive) {
            return;
        }

        const float SPEED = info->speed;

        if (grounded) {
            vel.x += SPEED * info->thrust;
            vel.x = clamp(vel.x, -SPEED, SPEED);
        } else {
            // soft clamp
            const float amount = SPEED * 0.01f;
            if (vel.x + amount <= SPEED) {
                vel.x += amount;
            }
        }
        facing = right;
        anim.set_animation("right");
    }

    void fly_towards(const glm::vec2& point, float speed_mul = 1.f) {
        glm::vec2 v = point - pos;
        // avoid divide by zero
        if (v.x == 0 && v.y == 0) {
            return;
        } else {
            glm::vec2 dir = glm::normalize(v);
            vel += dir * info->speed * speed_mul;
            vel.x = clamp(vel.x, -info->speed, info->speed);
            vel.y = clamp(vel.y, -info->speed, info->speed);

            if (dir.x < 0) {
                anim.set_animation("left");
            } else {
                anim.set_animation("right");
            }
        }
    }

    glm::vec2 interp_pos(double a) const {
        return glm::lerp(prev_pos, pos, (float)a);
    }

    rectangle worldspace_hitbox() const {
        return { (int)(pos.x - 8), (int)(pos.y - 8), 16, 16 };
        // TODO: maybe??
        //return { (int)(pos.x - 3), (int)(pos.y - 3), 6, 6 };
    }

    rectanglef worldspace_interp_rect(double a) const {
        auto p = interp_pos(a);
        return { p.x - 8.f, p.y - 8.f, 16.f, 16.f };
    }

    void init_position(float floor) {
        pos.y = floor;
        prev_pos = pos;
    }

    void warp(glm::vec2 pos_) {
        pos = prev_pos = pos_;
        vel = { 0, 0 };
    }
};


