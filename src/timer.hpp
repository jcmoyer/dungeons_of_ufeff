#pragma once

#include <cassert>
#include <cstdint>

struct timer
{
    uint32_t frame_start;
    uint32_t frame_end;

    bool expired(uint32_t current_frame) const
    {
        return current_frame >= frame_end;
    }

    double progress(uint32_t current_frame) const
    {
        assert(current_frame >= frame_start);
        return (current_frame - frame_start) / static_cast<double>(frame_end - frame_start);
    }
};
