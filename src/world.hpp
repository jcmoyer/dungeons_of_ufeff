#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "animation.hpp"
#include "animation_data.hpp"
#include "tilemap.hpp"

struct portal
{
    uint32_t world_x, world_y;
    std::string exit_map;
    std::string exit_name;
};

struct light
{
    float light_radius;
    float light_flicker_radius;
};

struct e_switch
{
    bool on = false;
};

enum move_state
{
    IDLE,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN
};

// informs animations and interactions
enum entity_type
{
    none,
    npc,
    door,
    et_portal,
    et_light,
    et_switch,
};

struct entity
{
    static constexpr uint32_t INVALID_SCRIPT = UINT32_MAX;
    static constexpr uint32_t INVALID_SPRITE = UINT32_MAX;

    std::string name;
    move_state mstate = IDLE;
    direction face = down;
    uint32_t world_x = 0, world_y = 0;
    uint32_t prev_world_x = 0, prev_world_y = 0;
    uint32_t move_speed = 4;
    uint32_t interact_script = INVALID_SCRIPT;
    uint32_t sprite_id = INVALID_SPRITE;
    entity_type type = none;
    bool open = false;
    portal portal_state;
    light light_state;
    e_switch switch_state;
    bool active = true;

    const animation_set* aset;
    animator anim;

    entity()
    {
    }

    bool is_interactive() const
    {
        return interact_script != INVALID_SCRIPT;
    }

    bool has_sprite() const
    {
        return sprite_id != INVALID_SPRITE;
    }

    void set_sprite_id(uint32_t id)
    {
        sprite_id = id;
        aset = get_animation_set(id);
        anim.set_animation_set(aset);
        if (type == npc)
        {
            set_animation_from_facing();
        }
        else if (type == door)
        {
            set_animation_from_doorstate();
        }
        else if (type == et_switch)
        {
            set_switchstate(switch_state.on);
            // set_animation_from_facing();
        }
    }

    void set_world_position(uint32_t new_world_x, uint32_t new_world_y)
    {
        assert(new_world_x % 16 == 0);
        assert(new_world_y % 16 == 0);
        world_x = new_world_x;
        world_y = new_world_y;
        prev_world_x = world_x;
        prev_world_y = world_y;
    }

    uint32_t tile_x() const
    {
        return world_x / 16;
    }

    uint32_t tile_y() const
    {
        return world_y / 16;
    }

    void update()
    {
        prev_world_x = world_x;
        prev_world_y = world_y;

        if (mstate == MOVE_LEFT)
        {
            world_x -= move_speed;
        }
        if (mstate == MOVE_RIGHT)
        {
            world_x += move_speed;
        }
        if (mstate == MOVE_UP)
        {
            world_y -= move_speed;
        }
        if (mstate == MOVE_DOWN)
        {
            world_y += move_speed;
        }

        switch (mstate)
        {
        case MOVE_LEFT:
        case MOVE_RIGHT:
            if (world_x % 16 == 0)
            {
                mstate = IDLE;
            }
            break;
        case MOVE_UP:
        case MOVE_DOWN:
            if (world_y % 16 == 0)
            {
                mstate = IDLE;
            }
            break;
        default:
            break;
        }

        if (has_sprite())
            anim.update();
    }

    void begin_move(direction f)
    {
        if (mstate != IDLE)
            return;

        switch (f)
        {
        case down:
            mstate = MOVE_DOWN;
            break;
        case left:
            mstate = MOVE_LEFT;
            break;
        case right:
            mstate = MOVE_RIGHT;
            break;
        case up:
            mstate = MOVE_UP;
            break;
        }

        set_facing(f);
    }

    void set_facing(direction f)
    {
        if (!has_sprite())
        {
            return;
        }
        // only NPCs have a meaningful facing direction
        if (type != npc)
        {
            return;
        }

        face = f;
        set_animation_from_facing();
    }

    void set_animation_from_facing()
    {
        switch (face)
        {
        case down:
            anim.set_animation("down");
            break;
        case left:
            anim.set_animation("left");
            break;
        case right:
            anim.set_animation("right");
            break;
        case up:
            anim.set_animation("up");
            break;
        }
    }

    void set_animation_from_doorstate()
    {
        const char* new_name = open ? "open" : "closed";
        anim.set_animation(new_name);
    }

    void set_doorstate(bool is_open)
    {
        if (!open && is_open)
        {
            anim.set_animation("closed-open");
        }
        open = is_open;
    }

    bool get_switchstate() const
    {
        return switch_state.on;
    }

    void toggle_switchstate()
    {
        set_switchstate(!switch_state.on);
    }

    void set_switchstate(bool is_on)
    {
        bool was_on = switch_state.on;
        bool now_on = is_on;

        switch_state.on = now_on;

        if (was_on && !now_on)
        {
            anim.set_animation("on-off");
        }
        else if (!was_on && now_on)
        {
            anim.set_animation("off-on");
        }
        else if (now_on)
        {
            // these last two branches look weird but they're here for switch initialization
            anim.set_animation("on");
        }
        else if (!now_on)
        {
            anim.set_animation("off");
        }
    }

    void set_active(bool is_active)
    {
        active = is_active;
    }
};

inline uint32_t tile_to_world(uint32_t x)
{
    return x * 16;
}

inline uint32_t world_to_tile(uint32_t x)
{
    return x / 16;
}

struct world
{
    static constexpr size_t INVALID_PLAYER_INDEX = SIZE_MAX;
    static constexpr uint32_t INVALID_ENCOUNTER = UINT32_MAX;

    tilemap map;
    std::vector<entity> ents;
    std::string map_name = "Unnamed Zone";
    std::string map_subtitle = "caption me";
    std::string music_name;
    float water_direction_x = 0;
    float water_direction_y = 0;
    float water_drift_x = 0;
    float water_drift_y = 0;
    float water_speed = 0;
    size_t player_index = INVALID_PLAYER_INDEX;
    bool dark = false;
    uint32_t encounter_set_id = INVALID_ENCOUNTER;
    std::string battle_field_name = "";

    bool has_encounters() const
    {
        return encounter_set_id != INVALID_ENCOUNTER;
    }

    std::size_t spawn_player(uint32_t tile_x, uint32_t tile_y)
    {
        assert(map.in_bounds(tile_x, tile_y));

        entity& e = ents.emplace_back();
        e.set_world_position(tile_to_world(tile_x), tile_to_world(tile_y));
        e.type = npc;
        e.set_sprite_id(0);

        player_index = ents.size() - 1;

        return player_index;
    }

    entity& player()
    {
        assert(player_index != INVALID_PLAYER_INDEX);
        return ents[player_index];
    }

    void update()
    {
        for (entity& e : ents)
        {
            e.update();
        }
    }

    entity* find_entity(const std::string& name)
    {
        for (size_t i = 0; i < ents.size(); ++i)
        {
            if (ents[i].name == name)
            {
                return &ents[i];
            }
        }
        return nullptr;
    }

    entity* entity_at(uint32_t tile_x, uint32_t tile_y)
    {
        if (!map.valid(tile_x, tile_y))
        {
            return nullptr;
        }

        uint32_t world_x = tile_to_world(tile_x);
        uint32_t world_y = tile_to_world(tile_y);

        for (entity& e : ents)
        {
            if (e.world_x == world_x && e.world_y == world_y)
            {
                return &e;
            }
        }

        return nullptr;
    }

    entity* portal_at(uint32_t tile_x, uint32_t tile_y)
    {
        if (!map.valid(tile_x, tile_y))
        {
            return nullptr;
        }

        uint32_t world_x = tile_to_world(tile_x);
        uint32_t world_y = tile_to_world(tile_y);

        for (entity& e : ents)
        {
            if (e.type == et_portal && e.world_x == world_x && e.world_y == world_y)
            {
                return &e;
            }
        }

        return nullptr;
    }
};

#include <unordered_map>
#include <variant>

using variant = std::variant<float, std::string, uint32_t>;

struct world_object
{
    std::string name;
    std::string type;
    uint32_t world_x, world_y;
    std::unordered_map<std::string, variant> props;
};

inline void handle_prop(world& w, const std::string& name, float val)
{
#define ASSIGN_PROP(n) \
    if (name == "" #n) \
    {                  \
        w.n = val;     \
    }
    ASSIGN_PROP(water_direction_x);
    ASSIGN_PROP(water_direction_y);
    ASSIGN_PROP(water_drift_x);
    ASSIGN_PROP(water_drift_y);
    ASSIGN_PROP(water_speed);
#undef ASSIGN_PROP
}

inline void handle_prop(world& w, const std::string& name, const std::string& val)
{
#define ASSIGN_PROP(n) \
    if (name == "" #n) \
    {                  \
        w.n = val;     \
    }
    ASSIGN_PROP(map_name);
    ASSIGN_PROP(map_subtitle);
    ASSIGN_PROP(music_name);
    ASSIGN_PROP(battle_field_name);
#undef ASSIGN_PROP
}

inline void handle_prop(world& w, const std::string& name, uint32_t val)
{
#define ASSIGN_PROP(n) \
    if (name == "" #n) \
    {                  \
        w.n = val;     \
    }
    ASSIGN_PROP(dark);
    ASSIGN_PROP(encounter_set_id);
#undef ASSIGN_PROP
}

inline uint8_t read_u8(std::ifstream& input)
{
    uint8_t val;
    input.read(reinterpret_cast<char*>(&val), sizeof(uint8_t));
    return val;
}

inline uint32_t read_u32(std::ifstream& input)
{
    uint32_t val;
    input.read(reinterpret_cast<char*>(&val), sizeof(uint32_t));
    return val;
}

inline float read_f32(std::ifstream& input)
{
    float val;
    input.read(reinterpret_cast<char*>(&val), sizeof(float));
    return val;
}

inline std::string read_string(std::ifstream& input)
{
    std::string str;
    uint32_t len = read_u32(input);
    str.resize(len);
    input.read(str.data(), len);
    return str;
}

inline variant read_variant(std::ifstream& input)
{
    uint8_t type = read_u8(input);
    if (type == 0)
    {
        float val = read_f32(input);
        return val;
    }
    else if (type == 1)
    {
        std::string val = read_string(input);
        return val;
    }
    else if (type == 2)
    {
        uint32_t val = read_u32(input);
        return val;
    }
    else
    {
        assert(0 && "bad prop type");
        return 0.f;
    }
}

#include <algorithm>

#include "mathutil.hpp"

inline world load_world(std::string_view filename)
{
    world wor;
    tilemap t;
    std::ifstream input(std::string(filename), std::ios::binary);

    input.read(reinterpret_cast<char*>(&t.width), sizeof(t.width));
    input.read(reinterpret_cast<char*>(&t.height), sizeof(t.height));

    const uint32_t size = t.width * t.height;
    t.base.resize(size);
    t.detail.resize(size);
    t.fringe.resize(size);

    input.read(reinterpret_cast<char*>(t.base.data()), t.base.size() * sizeof(tile));
    input.read(reinterpret_cast<char*>(t.detail.data()), t.detail.size() * sizeof(tile));
    input.read(reinterpret_cast<char*>(t.fringe.data()), t.fringe.size() * sizeof(tile));

    // fixup IDs from tiled export
    for (tile& x : t.base)
        --x.id;
    for (tile& x : t.detail)
        --x.id;
    for (tile& x : t.fringe)
        --x.id;

    wor.map = std::move(t);

    // read objects
    uint32_t obj_count = 0;
    input.read(reinterpret_cast<char*>(&obj_count), sizeof(obj_count));

    std::vector<world_object> objs;

    for (uint32_t i = 0; i < obj_count; ++i)
    {
        world_object& obj = objs.emplace_back();
        obj.name = read_string(input);
        obj.type = read_string(input);
        input.read(reinterpret_cast<char*>(&obj.world_x), sizeof(obj.world_x));
        input.read(reinterpret_cast<char*>(&obj.world_y), sizeof(obj.world_y));
        uint32_t prop_count = read_u32(input);
        for (uint32_t j = 0; j < prop_count; ++j)
        {
            std::string name = read_string(input);
            variant val = read_variant(input);
            obj.props[name] = val;
        }
    }

    for (const world_object& o : objs)
    {
        entity& e = wor.ents.emplace_back();
        e.name = o.name;
        if (o.type == "npc")
            e.type = npc;
        else if (o.type == "door")
            e.type = door;
        else if (o.type == "portal")
            e.type = et_portal;
        else if (o.type == "light")
            e.type = et_light;
        else if (o.type == "switch")
            e.type = et_switch;
        e.set_world_position(o.world_x, o.world_y);
        if (auto it = o.props.find("interact_id"); it != o.props.end())
        {
            e.interact_script = std::get<uint32_t>(it->second);
        }
        if (auto it = o.props.find("sprite_id"); it != o.props.end())
        {
            e.set_sprite_id(std::get<uint32_t>(it->second));
        }
        if (auto it = o.props.find("exit_map"); it != o.props.end())
        {
            e.portal_state.exit_map = std::get<std::string>(it->second);
        }
        if (auto it = o.props.find("exit_name"); it != o.props.end())
        {
            e.portal_state.exit_name = std::get<std::string>(it->second);
        }
        if (auto it = o.props.find("light_radius"); it != o.props.end())
        {
            e.light_state.light_radius = std::get<float>(it->second);
        }
        if (auto it = o.props.find("light_flicker_radius"); it != o.props.end())
        {
            e.light_state.light_flicker_radius = std::get<float>(it->second);
        }
        if (auto it = o.props.find("active"); it != o.props.end())
        {
            e.active = (bool)std::get<uint32_t>(it->second);
        }
        assert(e.world_x % 16 == 0);
        assert(e.world_y % 16 == 0);
    }

    uint32_t prop_count = read_u32(input);
    for (uint32_t i = 0; i < prop_count; ++i)
    {
        std::string name = read_string(input);
        variant val = read_variant(input);
        std::visit([&](auto&& arg) {
            handle_prop(wor, name, arg);
        },
                   val);
    }

    // TODO: maybe we want to do this to make cliffs have more depth?
    // t.bright_map.resize(size);
    // for (int y = 0; y < wor.map.height; ++y) {
    //    for (int x = 0; x < wor.map.width; ++x) {
    //        float& b = wor.map.bright_map[y * wor.map.width + x];
    //        if (wor.map.at(x, y).id != 39) {
    //            b = 1.0f;
    //        }
    //        if (b == 0.0f) {
    //            int ymin = wor.map.height, ymax = 0;
    //            // move upwards and look for end of sequence
    //            for (int y2 = y; y2 >= 0; --y2) {
    //                if (wor.map.at(x, y2).id == 39) {
    //                    ymin = std::min(ymin, y2);
    //                } else {
    //                    break;
    //                }
    //            }
    //            // move downwards and look for end of sequence
    //            for (int y2 = y; y2 < wor.map.height; ++y2) {
    //                if (wor.map.at(x, y2).id == 39) {
    //                    ymax = std::max(ymax, y2);
    //                } else {
    //                    break;
    //                }
    //            }
    //            for (int y2 = ymin; y2 <= ymax; ++y2) {
    //                const float BRIGHT_STEP = 0.11f;
    //                wor.map.bright_map[y2 * wor.map.width + x] = clamp(1.f - (y2 - ymin) * BRIGHT_STEP, 0.f, 1.f);
    //            }
    //        }
    //    }
    //}

    return wor;
}
