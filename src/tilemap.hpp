#pragma once

#include <cassert>
#include <fstream>
#include <string_view>
#include <vector>

#include "direction.hpp"

// clang-format off
enum collision_flags {
    from_none   = 0,
    from_right  = 0b0001,
    from_top    = 0b0010,
    from_left   = 0b0100,
    from_bottom = 0b1000,
    from_all    = 0b1111,
};

// how a tile restricts movement
enum movement_flags {
    move_none  = 0,
    move_right = 0b0001,
    move_up    = 0b0010,
    move_left  = 0b0100,
    move_down  = 0b1000,
    move_all   = 0b1111,
};
// clang-format on

inline movement_flags get_movement_flags(uint32_t tile_id)
{
    switch (tile_id)
    {
    case 128:
        return (movement_flags)(move_up | move_right | move_down);
    default:
        return move_all;
    }
}

inline collision_flags get_collision_flags(uint32_t tile_id)
{
    switch (tile_id)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 11:
    case 37:
    case 39:
    case 68:
    case 69:
    case 70:
    case 71:
    case 99:
    case 102:
    case 131:
    case 135:
    case 165:
    case 166:
    case 195:
    case 197:
    case 198:
    case 199:
    case 227:
    case 290:
    case 292:
    case 293:
    case 387:
        return from_all;
    case 128:
        return from_left;
    default:
        return from_none;
    }
}

struct tile
{
    uint32_t id;

    bool collides_from(direction d) const
    {
        collision_flags flags = get_collision_flags(id);
        switch (d)
        {
        case right:
            return flags & from_right;
        case left:
            return flags & from_left;
        case up:
            return flags & from_top;
        case down:
            return flags & from_bottom;
        }
        return false;
    }

    bool collides_any() const
    {
        return get_collision_flags(id) != from_none;
    }

    bool is_water() const
    {
        return id == 37;
    }

    bool is_waterfall() const
    {
        return id == 69;
    }

    bool is_lava() const
    {
        return id == 290;
    }

    bool invalid() const
    {
        return id == -1;
    }
};

struct tilemap
{
    uint32_t width, height;
    std::vector<tile> base;
    std::vector<tile> detail;
    std::vector<tile> fringe;
    // std::vector<float> bright_map;

    const tile& at(uint32_t x, uint32_t y) const
    {
        return base[y * width + x];
    }

    bool in_bounds(uint32_t x, uint32_t y) const
    {
        return x >= 0 && y >= 0 && x < width && y < height;
    }

    bool valid(uint32_t x, uint32_t y) const
    {
        bool in_bounds = x >= 0 && y >= 0 && x < width && y < height;
        if (!in_bounds)
        {
            return false;
        }
        return !base[y * width + x].invalid();
    }

    bool collides_from(uint32_t x, uint32_t y, direction d) const
    {
        if (!in_bounds(x, y))
        {
            return true;
        }
        return base[y * width + x].collides_from(d) || detail[y * width + x].collides_from(d);
    }

    bool can_move_from(uint32_t x, uint32_t y, direction d) const
    {
        if (!in_bounds(x, y))
        {
            return false;
        }

        movement_flags flags = (movement_flags)(get_movement_flags(base[y * width + x].id) & get_movement_flags(detail[y * width + x].id));

        switch (d)
        {
        case left:
            return flags & move_left;
        case right:
            return flags & move_right;
        case up:
            return flags & move_up;
        case down:
            return flags & move_down;
        default:
            return false;
        }
    }
};
