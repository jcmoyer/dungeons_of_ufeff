#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "rectangle.hpp"

struct animation_frame
{
    rectangle rect;
    uint32_t frame_time;
};

struct animation
{
    std::vector<animation_frame> frames;
    std::string next_animation;
};

struct animation_set
{
    std::unordered_map<std::string, animation> anims;
};

struct animator
{
    const animation_set* aset;
    std::string current_name;
    size_t frame_index = 0;
    uint32_t counter = 0;

    const animation& current_animation() const
    {
        return aset->anims.at(current_name);
    }

    const animation_frame& current_frame() const
    {
        return current_animation().frames[frame_index];
    }

    const rectangle& current_rect() const
    {
        return current_frame().rect;
    }

    void update()
    {
        ++counter;
        if (counter >= current_frame().frame_time)
        {
            counter = 0;
            ++frame_index;

            if (frame_index == current_animation().frames.size())
            {
                frame_index = 0;
                if (aset->anims.count(current_animation().next_animation))
                {
                    current_name = current_animation().next_animation;
                }
            }
        }
    }

    void reset()
    {
        counter = 0;
        frame_index = 0;
    }

    void set_animation_set(const animation_set* a)
    {
        aset = a;
    }

    void set_animation(const std::string& name)
    {
        if (current_name == name)
        {
            return;
        }

        if (aset->anims.count(name))
        {
            current_name = name;
            reset();
        }
    }
};
