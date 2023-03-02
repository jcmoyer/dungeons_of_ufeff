#pragma once

#include <glm/vec2.hpp>
#include "../animation.hpp"
#include "../tilemap.hpp"
#include "../rectangle.hpp"
#include "battle_field.hpp"
#include <glm/gtx/compatibility.hpp>

struct battle_projectile;

struct battle_projectile_behavior {
    using update_function = void(*)(battle_field& field, battle_projectile& self);

    glm::vec2 acceleration{};
    update_function update = nullptr;
};

struct battle_projectile {
    size_t owner = -1;

    glm::vec2 pos, prev_pos, vel;
    animator anim;
    bool alive = true;
    int collision_size = 4;
    uint32_t pierce = 0;
    direction dir = right;
    battle_projectile_behavior* behavior = nullptr;
    float angle = 0, prev_angle = 0;
    float angular_velocity = 0;
    bool angle_to_velocity = false;
    int32_t power = 1;

    battle_projectile(const animation_set* aset) {
        anim.set_animation_set(aset);
    }

    void kill() {
        alive = false;
    }

    void update(battle_field& field) {
        prev_pos = pos;
        pos += vel;
        prev_angle = angle;
        

        if (angle_to_velocity) {
            angle = std::atan2(vel.y, vel.x);
        } else {
            angle += angular_velocity;
        }

        if (behavior) {
            vel += behavior->acceleration;

            if (behavior->update) {
                behavior->update(field, *this);
            }
        }

        anim.update();
    }

    glm::vec2 interp_pos(double a) const {
        return glm::lerp(prev_pos, pos, (float)a);
    }

    rectangle worldspace_hitbox() const {
        // TODO differing size
        return { (int)(pos.x - collision_size / 2), (int)(pos.y - collision_size / 2), collision_size, collision_size };
    }

    rectanglef worldspace_interp_rect(double a) const {
        auto p = interp_pos(a);
        return { p.x - 8.f, p.y - 8.f, 16.f, 16.f };
    }

    float interp_angle(double a) const {
        return (float)lerp<double>(prev_angle, angle, a);
    }
};
