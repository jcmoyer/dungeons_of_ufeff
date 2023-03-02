#pragma once

#include <glm/vec2.hpp>

#include "../animation.hpp"
#include "../direction.hpp"

struct battle_fx
{
    size_t owner = SIZE_MAX;
    glm::vec2 pos{}, prev_pos{}, vel{};
    animator anim;
    bool alive = true;
    direction dir;

    battle_fx(uint32_t sprite_id, direction d)
    {
        anim.set_animation_set(get_animation_set(sprite_id));
        dir = d;
    }

    void update()
    {
        prev_pos = pos;
        pos += vel;
        anim.update();

        // HACK
        if (anim.current_name == "dead")
        {
            alive = false;
        }
    }

    void attach(size_t owner_id)
    {
        owner = owner_id;
    }

    bool attached() const
    {
        return owner != SIZE_MAX;
    }

    glm::vec2 interp_pos(double a) const
    {
        return glm::lerp(prev_pos, pos, (float)a);
    }

    rectanglef worldspace_interp_rect(double a) const
    {
        auto p = interp_pos(a);
        return {p.x - 8.f, p.y - 8.f, 16.f, 16.f};
    }
};

struct battle_fx_system
{
    std::vector<battle_fx> effects;

    battle_fx& spawn(uint32_t sprite_id, direction d, glm::vec2 pos)
    {
        auto& fx = effects.emplace_back(sprite_id, d);
        fx.pos = fx.prev_pos = pos;
        return fx;
    }

    void update()
    {
        for (auto& fx : effects)
        {
            fx.update();
        }

        if (effects.size())
        {
            auto remove_first = std::remove_if(effects.begin(), effects.end(), [](const battle_fx& p) {
                return !p.alive;
            });
            effects.erase(remove_first, effects.end());
        }
    }
};
