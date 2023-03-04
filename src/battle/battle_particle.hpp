#pragma once

#include <glm/common.hpp>
#include <glm/vec2.hpp>

#include "../animation.hpp"
#include "../animation_data.hpp"

struct battle_particle
{
    glm::vec2 pos{}, prev_pos{}, vel{};
    animator anim;
    uint32_t life = 0;
    uint32_t max_life = 0;
    bool alive = true;
    float size = 0.f;  // size over life
    float scale = 1.f; // overall scaling
    glm::vec2 acc{};

    battle_particle(uint32_t sprite_id)
    {
        anim.set_animation_set(get_animation_set(sprite_id));
    }

    void set_life(uint32_t frames)
    {
        life = max_life = frames;
        size = 1.f;
    }

    void update(float floor)
    {
        (void)floor;

        float factor = scale * size;
        float actual_size = factor * 16.f;
        float actual_floor = actual_size / 2;

        prev_pos = pos;
        pos += vel;
        vel += acc;
        if (pos.y <= actual_floor)
        {
            pos.y = actual_floor;
            vel.x *= 0.3f;
        }
        if (life)
            --life;
        alive = life;
        size = life / (float)max_life;
        anim.update();
    }

    glm::vec2 interp_pos(double a) const
    {
        return glm::mix(prev_pos, pos, (float)a);
    }

    rectanglef worldspace_interp_rect(double a) const
    {
        auto p = interp_pos(a);
        return {p.x - 8.f * scale * size, p.y - 8.f * scale * size, scale * size * 16.f, scale * size * 16.f};
    }
};

struct battle_particle_system
{
    std::vector<battle_particle> particles;

    void update(float floor)
    {
        for (auto& p : particles)
        {
            p.update(floor);
        }

        if (particles.size())
        {
            auto remove_first = std::remove_if(particles.begin(), particles.end(), [](const battle_particle& p) {
                return !p.alive;
            });
            particles.erase(remove_first, particles.end());
        }
    }

    battle_particle& emit(uint32_t sprite_id, glm::vec2 pos, glm::vec2 vel, float scale)
    {
        battle_particle& p = particles.emplace_back(sprite_id);
        p.prev_pos = p.pos = pos;
        p.set_life(30);
        p.vel = vel;
        p.scale = scale;
        return p;
    }
};