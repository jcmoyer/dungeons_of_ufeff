#include "st_play.hpp"

#include <GL/gl3w.h>
#include <glm/gtx/compatibility.hpp>
#include <vector>

#include "audio.hpp"
#include "bmfont.hpp"
#include "foam_emitter.hpp"
#include "game.hpp"
#include "gamestate.hpp"
#include "mathutil.hpp"
#include "npc.hpp"
#include "random_vec.hpp"
#include "spritebatch.hpp"
#include "texture_manager.hpp"
#include "water_renderer.hpp"
#include "world.hpp"

struct point
{
    int x, y;
};

point offset(direction f)
{
    switch (f)
    {
    case left:
        return {-1, 0};
    case right:
        return {1, 0};
    case up:
        return {0, -1};
    case down:
        return {0, 1};
    default:
        assert(false && "bad direction value");
        return {0, 0};
    }
}

bool try_move_ent(world& wor, entity& e, direction f)
{
    uint32_t current_x, current_y;
    uint32_t target_x, target_y;
    direction enter_from;

    // ignore move if the entity is already moving
    if (e.mstate != IDLE)
    {
        return false;
    }

    current_x = e.tile_x();
    current_y = e.tile_y();

    switch (f)
    {
    case left:
        target_x = e.tile_x() - 1;
        target_y = e.tile_y();
        enter_from = right;
        break;
    case right:
        target_x = e.tile_x() + 1;
        target_y = e.tile_y();
        enter_from = left;
        break;
    case up:
        target_x = e.tile_x();
        target_y = e.tile_y() - 1;
        enter_from = down;
        break;
    case down:
        target_x = e.tile_x();
        target_y = e.tile_y() + 1;
        enter_from = up;
        break;
    }

    // don't attempt to move to non-existent tiles
    if (!wor.map.valid(target_x, target_y))
    {
        e.set_facing(f);
        return false;
    }

    const tile& t = wor.map.at(target_x, target_y);

    if (wor.map.collides_from(target_x, target_y, enter_from))
    {
        e.set_facing(f);
        return false;
    }

    // ensure we can move in the desired direction
    if (!wor.map.can_move_from(current_x, current_y, invert(enter_from)))
    {
        e.set_facing(f);
        return false;
    }

    // don't move to tiles that have a visible entity
    if (entity* other = wor.entity_at(target_x, target_y); other)
    {
        if (!other->active)
        {
            // completely ignore inactive ents
        }
        else if (other->type == npc || other->type == et_switch)
        {
            e.set_facing(f);
            return false;
        }
        else if (other->type == door && !other->open)
        {
            e.set_facing(f);
            return false;
        }
        else if (other->type == et_light && other->has_sprite())
        {
            e.set_facing(f);
            return false;
        }
    }

    e.begin_move(f);
    return true;
}

st_play::st_play(game* g, shared_state* s)
    : owner{g}, state{s}
{
}

void st_play::init()
{
    t_atlas = state->texman->get("assets/amalgamation.png");
    t_water_base = state->texman->get("assets/water_base.png");
    t_water_foam = state->texman->get("assets/water_foam.png");
    t_waterfall = state->texman->get("assets/waterfall.png");
    t_dialogue_back = state->texman->get("assets/dialogue_background.png");
    t_light_mask = state->texman->get("assets/mask.png");
    t_lava_base = state->texman->get("assets/lava_base.png");
    t_lava_blend = state->texman->get("assets/lava_blend.png");

    cam.set_bounds(0, 0, 100 * 16, 100 * 16);
    cam.set_width(INTERNAL_WIDTH);
    cam.set_height(INTERNAL_HEIGHT);

    wor = load_world("assets/maps/hub.bin");
    current_map_name = "hub";

    entity* playerspawn = wor.find_entity("playerspawn");
    assert(playerspawn != nullptr);

    player_handle = wor.spawn_player(playerspawn->tile_x(), playerspawn->tile_y());

    cam.center_on(wor.player().world_x + 8, wor.player().world_y + 8);
    prev_cam = cam;

    water_render = std::make_unique<water_renderer>();
    water_render->set_output_dimensions(INTERNAL_WIDTH, INTERNAL_HEIGHT); // TODO: get from game

    foam_em = std::make_unique<foam_emitter>(state->quad_render);
}

void st_play::handle_event(const SDL_Event& ev)
{
    //if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_F5) {
    //    begin_battle_transition(pick_random_map_encounter());
    //}
    //if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_F6) {
    //    sub = none;
    //}

    if (sub == map_intro_fadein || sub == map_intro_title || sub == map_intro_subtitle)
    {
        // don't process events during fadein
        return;
    }

    if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == state->session->keybinds.get_key(IA_INTERACT))
    {
        if (sub == none)
        {
            entity& player = wor.player();
            point d = offset(player.face);
            entity* e = wor.entity_at(
                player.tile_x() + d.x,
                player.tile_y() + d.y);

            if (!e)
            {
                return;
            }

            if (e->is_interactive() && e->active)
            {
                auto script = get_npc_interact_script(e->interact_script);
                e->set_facing(invert(player.face));
                auto context = st_interact_context(this, e);
                script(context);
            }
        }
        else if (sub == message)
        {
            sub = none;
        }
    }

    if (sub == none)
    {
        if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
        {
            owner->transition(transition_to::options);
        }
    }
}

void st_play::update()
{
    if (wor.dark)
    {
        owner->set_mask_effect(1.f);
    }
    else
    {
        owner->set_mask_effect(0.f);
    }

    if (sub == map_intro_title)
    {
        if (mi_timer.expired(state->frame_counter))
        {
            sub = map_intro_subtitle;
            mi_timer = owner->create_timer(3);
        }
    }
    else if (sub == map_intro_subtitle)
    {
        if (mi_timer.expired(state->frame_counter))
        {
            sub = map_intro_fadein;
            mi_timer = owner->create_timer(1);
        }
    }
    else if (sub == map_intro_fadein)
    {
        if (mi_timer.expired(state->frame_counter))
        {
            sub = none;
        }
    }
    else if (sub == map_exit_fadeout)
    {
        if (me_timer.expired(state->frame_counter))
        {
            current_music->done = true;
            wor = std::move(me_new_world);
            begin_map_intro();
        }
        else
        {
            current_music->volume = 1.0f - static_cast<float>(me_timer.progress(state->frame_counter));
        }
    }
    else if (sub == battle_fadeout)
    {
        if (b_fade_timer.expired(state->frame_counter))
        {
            transition_particles.clear();
            current_music->paused = true;
            assert(wor.has_encounters());
            assert(wor.battle_field_name.size());
            owner->get_battle_state().set_encounter(b_next_encounter);
            owner->get_battle_state().set_battle_field_name(wor.battle_field_name);
            owner->transition(transition_to::battle);
        }
        else
        {
            current_music->volume = 1.0f - static_cast<float>(b_fade_timer.progress(state->frame_counter));
        }
        for (auto& p : transition_particles)
        {
            p.prev_pos = p.pos;
            p.pos += p.vel;
        }
    }
    else if (sub == battle_fadein)
    {
        if (b_fade_timer.expired(state->frame_counter))
        {
            current_music->volume = 1.f;
            sub = none;
        }
        else
        {
            current_music->volume = static_cast<float>(b_fade_timer.progress(state->frame_counter));
        }
    }
    else if (sub == map_local_portal_fadeout)
    {
        if (mlp_fade_timer.expired(state->frame_counter))
        {
            wor.player().set_world_position(mlp_exit_x, mlp_exit_y);
            mlp_fade_timer = owner->create_timer(1);
            sub = map_local_portal_fadein;
        }
    }
    else if (sub == map_local_portal_fadein)
    {
        if (mlp_fade_timer.expired(state->frame_counter))
        {
            sub = none;
        }
    }

    if (sub == none)
    {
        if (message_queue.size() > 0)
        {
            sub = message;
            display_next_message();
        }

        process_scheduled_scripts();
    }

    if (sub == none)
    {
        if (state->input->keys[SDLK_LEFT])
        {
            try_move_player(left);
        }
        else if (state->input->keys[SDLK_RIGHT])
        {
            try_move_player(right);
        }
        else if (state->input->keys[SDLK_UP])
        {
            try_move_player(up);
        }
        else if (state->input->keys[SDLK_DOWN])
        {
            try_move_player(down);
        }
    }

    const float FOAM_EMIT_RATE = 0.15f;
    for (uint32_t y = 0; y < wor.map.height; ++y)
    {
        for (uint32_t x = 0; x < wor.map.width; ++x)
        {
            const tile& t = wor.map.at(x, y);
            if (t.is_water())
            {
                if (wor.map.valid(x - 1, y) && !wor.map.at(x - 1, y).is_water() && random::chance(FOAM_EMIT_RATE))
                {
                    foam_em->emit({x * 16, y * 16 + random::rand_int(0, 16)}, rand_vec2(-0.8f, 0.0f, -0.5f, 0.0f));
                }
                if (wor.map.valid(x + 1, y) && !wor.map.at(x + 1, y).is_water() && random::chance(FOAM_EMIT_RATE))
                {
                    foam_em->emit({16 + x * 16, y * 16 + random::rand_int(0, 16)}, rand_vec2(0.0f, 0.8f, -0.5f, 0.0f));
                }
                if (wor.map.valid(x, y - 1) && !wor.map.at(x, y - 1).is_water() && random::chance(FOAM_EMIT_RATE))
                {
                    foam_em->emit({x * 16 + random::rand_int(0, 16), y * 16}, rand_vec2(-0.2f, 0.2f, -0.8f, 0.0f));
                }
                if (wor.map.valid(x, y - 1) && wor.map.at(x, y - 1).is_waterfall() && random::chance(3 * FOAM_EMIT_RATE))
                {
                    foam_em->emit({x * 16 + random::rand_int(0, 16), y * 16}, rand_vec2(-0.2f, 0.2f, -0.8f, 0.0f), 6.f);
                }
                if (wor.map.valid(x, y + 1) && !wor.map.at(x, y + 1).is_water() && random::chance(FOAM_EMIT_RATE))
                {
                    foam_em->emit({x * 16 + random::rand_int(0, 16), 16 + y * 16}, rand_vec2(-0.2f, 0.2f, 0.0f, 0.8f));
                }
            }
        }
    }
    foam_em->update();

    wor.update();

    prev_cam = cam;
    cam.center_on(wor.player().world_x + 8, wor.player().world_y + 8);
}

void st_play::render(double a)
{
    // only drawing 2d stuff here
    glDisable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    camera lerp_cam;
    lerp_cam.set_x((int)lerp<double>(prev_cam.get_view().x, cam.get_view().x, a));
    lerp_cam.set_y((int)lerp<double>(prev_cam.get_view().y, cam.get_view().y, a));
    lerp_cam.set_width(cam.get_view().w);
    lerp_cam.set_height(cam.get_view().h);
    lerp_cam.set_bounds(cam.get_bounds());
    lerp_cam.clamp_to_bounds();

    glBindTexture(GL_TEXTURE_2D, t_atlas->tex);

    water_cmds.clear();
    waterfall_cmds.clear();
    lava_cmds.clear();
    light_cmds.clear();

    state->batch->begin();
    int min_tile_x = lerp_cam.left() / 16;
    int max_tile_x = std::min(1 + lerp_cam.right() / 16, (int)wor.map.width);
    int min_tile_y = lerp_cam.top() / 16;
    int max_tile_y = std::min(1 + lerp_cam.bottom() / 16, (int)wor.map.height);
    for (int y = min_tile_y; y < max_tile_y; ++y)
    {
        for (int x = min_tile_x; x < max_tile_x; ++x)
        {
            tile& t = wor.map.base.at(y * wor.map.width + x);
            uint32_t tid = t.id;
            if (tid == -1)
                continue;
            int ysrc = static_cast<int>(tid / 32);
            int xsrc = static_cast<int>(tid % 32);
            rectangle src = {xsrc * 16, ysrc * 16, 16, 16};
            rectangle dest{x * 16 - lerp_cam.left(), y * 16 - lerp_cam.top(), 16, 16};
            if (t.is_water())
            {
                // we want to draw water with a different shader
                water_cmds.push_back({dest, (float)x * 16, (float)y * 16});
            }
            else if (t.is_waterfall())
            {
                waterfall_cmds.push_back({dest, (float)x * 16, (float)y * 16});
            }
            else if (t.is_lava())
            {
                lava_cmds.push_back({dest, (float)x * 16, (float)y * 16});
            }
            else
            {
                // immediate draw
                //float brightness = wor.map.bright_map[y * wor.map.width + x];
                state->batch->draw_quad(t_atlas, src, dest);
            }
        }
    }
    state->batch->end();

    glBindTexture(GL_TEXTURE_2D, t_water_base->tex);

    const double water_time = state->frame_counter / 30.0;

    water_render_parameters water_params;
    water_params.blend_amount = 0.0;
    water_params.global_time = water_time;
    water_params.water_direction = {wor.water_direction_x, wor.water_direction_y};
    water_params.water_drift_range = {wor.water_drift_x, wor.water_drift_y};
    water_params.water_drift_scale = {32, 16};
    water_params.water_speed = wor.water_speed;

    water_render_parameters foam_params = water_params;
    foam_params.water_speed *= 2.f;

    water_render_parameters waterfall_params = water_params;
    waterfall_params.water_direction = {0, 1};
    waterfall_params.water_speed = 1.0;
    waterfall_params.water_drift_range = {0, 0};

    water_render_parameters lava_params = water_params;
    lava_params.blend_amount = 0.5f + 0.5f * sin(water_time * 2.f);
    lava_params.water_drift_scale = {16, 16};

    water_render->begin(water_params);
    for (const auto& cmd : water_cmds)
    {
        water_render->draw_quad(t_water_base, {0, 0, 16, 16}, cmd.dest, cmd.world_x, cmd.world_y, 16);
    }
    water_render->end();

    glBindTexture(GL_TEXTURE_2D, t_water_foam->tex);
    water_render->begin(foam_params);
    for (const auto& cmd : water_cmds)
    {
        water_render->draw_quad(t_water_foam, {0, 0, 16, 16}, cmd.dest, cmd.world_x, cmd.world_y, 16);
    }
    water_render->end();

    glBindTexture(GL_TEXTURE_2D, t_waterfall->tex);
    water_render->begin(waterfall_params);
    for (const auto& cmd : waterfall_cmds)
    {
        water_render->draw_quad(t_waterfall, {0, 0, 16, 16}, cmd.dest, cmd.world_x, cmd.world_y, 16);
    }
    water_render->end();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t_lava_base->tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, t_lava_blend->tex);

    water_render->begin(lava_params);
    for (const auto& cmd : lava_cmds)
    {
        water_render->draw_quad(t_lava_base, {0, 0, 16, 16}, cmd.dest, cmd.world_x, cmd.world_y, 16);
    }
    water_render->end();

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, t_atlas->tex);

    state->batch->begin();
    for (int y = min_tile_y; y < max_tile_y; ++y)
    {
        for (int x = min_tile_x; x < max_tile_x; ++x)
        {
            tile& t = wor.map.detail.at(y * wor.map.width + x);
            uint32_t tid = t.id;
            if (tid == -1)
                continue;
            int ysrc = static_cast<int>(tid / 32);
            int xsrc = static_cast<int>(tid % 32);
            rectangle src = {xsrc * 16, ysrc * 16, 16, 16};
            rectangle dest{x * 16 - lerp_cam.left(), y * 16 - lerp_cam.top(), 16, 16};
            state->batch->draw_quad(t_atlas, src, dest);
        }
    }
    state->batch->end();

    state->batch->begin();
    for (entity& e : wor.ents)
    {
        if (e.type == et_light)
        {
            // intentionally using frame_counter here to make the flickering more chaotic,
            // but still deterministic
            // also seeded by world position so different light sources aren't synchronized
            const float a = static_cast<float>(e.world_x + e.world_y + state->frame_counter);
            float d_flicker = e.light_state.light_flicker_radius * cos(a);

            light_cmds.push_back({static_cast<int>(e.world_x) - lerp_cam.left(), static_cast<int>(e.world_y) - lerp_cam.top(), d_flicker + e.light_state.light_radius});
        }

        if (!e.active || !e.has_sprite())
        {
            continue;
        }

        rectangle src = e.anim.current_rect();

        int e_lerp_x = static_cast<int>(lerp(
            static_cast<double>(e.prev_world_x),
            static_cast<double>(e.world_x),
            a));

        int e_lerp_y = static_cast<int>(lerp(
            static_cast<double>(e.prev_world_y),
            static_cast<double>(e.world_y),
            a));

        int dx = e_lerp_x - lerp_cam.left();
        int dy = e_lerp_y - lerp_cam.top();

        rectangle dest{dx, dy, 16, 16};
        state->batch->draw_quad(t_atlas, src, dest);

        if (&wor.player() == &e && wor.dark)
        {
            light_cmds.push_back({dx, dy, 1.f});
        }
    }
    state->batch->end();

    foam_em->render(lerp_cam);

    state->batch->begin();
    for (int y = min_tile_y; y < max_tile_y; ++y)
    {
        for (int x = min_tile_x; x < max_tile_x; ++x)
        {
            tile& t = wor.map.fringe.at(y * wor.map.width + x);
            uint32_t tid = t.id;
            if (tid == -1)
                continue;
            int ysrc = static_cast<int>(tid / 32);
            int xsrc = static_cast<int>(tid % 32);
            rectangle src = {xsrc * 16, ysrc * 16, 16, 16};
            rectangle dest{x * 16 - lerp_cam.left(), y * 16 - lerp_cam.top(), 16, 16};
            state->batch->draw_quad(t_atlas, src, dest);
        }
    }
    state->batch->end();

    if (wor.dark)
    {
        owner->render_to_mask();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, t_light_mask->tex);

        state->batch->begin();
        for (const auto& cmd : light_cmds)
        {
            rectangle dest{cmd.world_x, cmd.world_y, 16, 16};
            dest.inflate(static_cast<int>(cmd.radius * 16.f), static_cast<int>(cmd.radius * 16));
            state->batch->draw_quad(t_light_mask, {0, 0, t_light_mask->width, t_light_mask->height}, dest);
        }

        state->batch->end();
    }

    owner->apply_mask();
    owner->clear_mask();
    owner->render_to_scene();

    // render ui elements on top
    if (sub == message)
    {
        render_dialogue_box(m_info, *state->batch, m_text, INTERNAL_WIDTH / 2 - m_rect.w / 2, INTERNAL_HEIGHT / 2 - m_rect.h / 2);
    }

    render_map_intro();
    render_map_fadeout();
    render_battle_fade(a);
    render_mlp_fade();
}

void st_play::process_scheduled_scripts()
{
    if (scheduled_scripts.size() == 0)
    {
        return;
    }

    // scripts can append more scripts to the collection, invalidating iterators; we have to take a different approach
    for (size_t i = scheduled_scripts.size() - 1; i != -1; --i)
    {
        if (scheduled_scripts[i].t.expired(state->frame_counter))
        {
            auto script = get_npc_interact_script(scheduled_scripts[i].script_id);
            auto context = st_interact_context(this, nullptr);
            script(context);
            scheduled_scripts.erase(scheduled_scripts.begin() + i);
        }
    }
}

void st_play::display_next_message()
{
    m_text = message_queue.front();
    m_info = {t_atlas, t_dialogue_back};
    m_rect = measure_dialogue_box(m_info, m_text);
    message_queue.pop_front();

    // UBERHACK
    // need to implement a proper scripting system with coroutines so we can actually suspend/resume scripts
    if (m_text == "#ragworm")
    {
        begin_battle_transition(get_final_boss_encounter());
    }
}

void st_play::show_message(std::string_view m)
{
    message_queue.push_back(std::string(m));
}

void st_play::enter(gamestate* old)
{
    // just got out of a battle
    if (sub == battle_fadeout)
    {
        steps = 0;
        // just got out of final boss
        if (b_next_encounter == get_final_boss_encounter())
        {
            owner->transition(transition_to::gamewin);
        }
        else
        {
            sub = battle_fadein;
            current_music->paused = false;
            b_fade_timer = owner->create_timer(1);
        }
    }
    else if (old == &owner->get_options_state())
    {
        // do nothing
    }
    else
    {
        steps = 0;
        begin_map_intro();
    }
}

void st_play::leave()
{
}

void st_play::render_battle_fade(double a)
{
    if (!(sub == battle_fadeout || sub == battle_fadein))
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, t_atlas->tex);
    state->batch->begin();
    for (int i = 0; i < transition_particles.size(); ++i)
    {
        glm::vec2 lerp_pos = glm::lerp(transition_particles[i].prev_pos, transition_particles[i].pos, (float)a);
        state->batch->draw_quad(t_atlas, {144, 128, 16, 16}, {(int)lerp_pos.x, (int)lerp_pos.y, 32, 32});
    }
    state->batch->end();

    owner->render_to_scene();

    const float out = clamp(static_cast<float>(b_fade_timer.progress(state->frame_counter)), 0.0f, 1.0f);
    const float in = 1.0f - out;
    const float fade = sub == battle_fadeout ? out : in;

    state->quad_render->begin();
    state->quad_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT}, 0, 0, 0, fade);
    state->quad_render->end();
}

void st_play::render_map_fadeout()
{
    if (sub != map_exit_fadeout)
    {
        return;
    }

    const float c = clamp(static_cast<float>(me_timer.progress(state->frame_counter)), 0.0f, 1.0f);

    state->quad_render->begin();
    state->quad_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT}, 0, 0, 0, c);
    state->quad_render->end();
}

void st_play::render_map_intro()
{
    if (!(sub == map_intro_title || sub == map_intro_subtitle || sub == map_intro_fadein))
    {
        return;
    }

    state->font->set_texture(state->texman->get("assets/amalgamation.png"));

    const float c = clamp(static_cast<float>(mi_timer.progress(state->frame_counter)), 0.0f, 1.0f);
    const float x = 1.0f - c;

    auto m1 = state->font->measure_string(wor.map_name, glyph_map_font_blue_large::instance());
    auto m2 = state->font->measure_string(wor.map_subtitle, glyph_map_font_white_small::instance());

    const int TITLE_X = INTERNAL_WIDTH / 2 - m1.width / 2;
    const int TITLE_Y = INTERNAL_HEIGHT / 2 - m1.height;
    const int SUBTITLE_X = INTERNAL_WIDTH / 2 - m2.width / 2;
    const int SUBTITLE_Y = INTERNAL_HEIGHT / 2 + m2.height;

    if (sub == map_intro_title)
    {
        state->quad_render->begin();
        state->quad_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT}, 0, 0, 0, 1);
        state->quad_render->end();

        state->font->begin(state->batch);
        state->font->draw_string(glyph_map_font_blue_large::instance(), wor.map_name, TITLE_X, TITLE_Y, c, c, c);
        state->font->end();
    }

    else if (sub == map_intro_subtitle)
    {
        state->quad_render->begin();
        state->quad_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT}, 0, 0, 0, 1);
        state->quad_render->end();

        state->font->begin(state->batch);
        state->font->draw_string(glyph_map_font_blue_large::instance(), wor.map_name, TITLE_X, TITLE_Y);
        state->font->draw_string(glyph_map_font_white_small::instance(), wor.map_subtitle, SUBTITLE_X, SUBTITLE_Y, c, c, c);
        state->font->end();
    }

    else if (sub == map_intro_fadein)
    {
        state->quad_render->begin();
        state->quad_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT}, 0, 0, 0, x);
        state->quad_render->end();

        state->font->begin(state->batch);
        state->font->draw_string(glyph_map_font_blue_large::instance(), wor.map_name, TITLE_X, TITLE_Y, 1, 1, 1, x);
        state->font->draw_string(glyph_map_font_white_small::instance(), wor.map_subtitle, SUBTITLE_X, SUBTITLE_Y, 1, 1, 1, x);
        state->font->end();
    }
}

void st_play::begin_map_transition(const std::string& map_name, const std::string& exit_name)
{
    if (map_name == current_map_name)
    {

        entity* exit = wor.find_entity(exit_name);
        assert(exit && "no exit with matching name");

        // we don't want to transition immediately because the player is still in motion to the local portal
        // this is kind of weird and causes some timing issues (like this); ideally the engine would process
        // portal AFTER the player has stepped onto them
        mlp_exit_x = exit->world_x;
        mlp_exit_y = exit->world_y;

        sub = map_local_portal_fadeout;
        mlp_fade_timer = owner->create_timer(1);

        return;
    }

    scheduled_scripts.clear();
    current_map_name = map_name;

    const std::string filename = fmt::format("assets/maps/{}.bin", map_name);
    me_new_world = load_world(filename.c_str());
    entity* spawn = me_new_world.find_entity(exit_name);
    assert(spawn && "no exit with matching name");
    me_new_world.spawn_player(spawn->tile_x(), spawn->tile_y());

    me_timer = owner->create_timer(1);
    sub = map_exit_fadeout;
}

void st_play::begin_map_intro()
{
    mi_timer = owner->create_timer(3);
    sub = map_intro_title;

    if (wor.music_name.size())
    {
        std::string music_filename = fmt::format("assets/music/{}.ogg", wor.music_name);
        current_music = state->audio->play_music(music_filename.c_str());
    }
}

void st_play::begin_battle_transition(encounter enc)
{
    state->audio->play_sound("assets/sound/ambush.ogg");
    b_fade_timer = owner->create_timer(1);
    sub = battle_fadeout;
    b_next_encounter = std::move(enc);

    emit_battle_transition_particles();
}

void st_play::render_mlp_fade()
{
    if (!(sub == map_local_portal_fadein || sub == map_local_portal_fadeout))
    {
        return;
    }

    const float out = clamp(static_cast<float>(mlp_fade_timer.progress(state->frame_counter)), 0.0f, 1.0f);
    const float in = 1.0f - out;

    state->quad_render->begin();
    state->quad_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT}, 0, 0, 0, sub == map_local_portal_fadein ? in : out);
    state->quad_render->end();
}

encounter st_play::pick_random_map_encounter()
{
    return get_encounter_set(wor.encounter_set_id).random_encounter();
}

void st_play::emit_battle_transition_particles()
{
    transition_particles.clear();
    for (int i = 0; i < 200; ++i)
    {
        battle_transition_particle& p = transition_particles.emplace_back();
        //p.pos = p.prev_pos = { INTERNAL_WIDTH + random::rand_real(0, INTERNAL_WIDTH), INTERNAL_HEIGHT + random::rand_real(0, INTERNAL_HEIGHT) };
        //p.vel = rand_vec2(-2, -1, -1, -0.3) * 16.f;
        p.pos = p.prev_pos = {random::rand_real(0, INTERNAL_WIDTH), INTERNAL_HEIGHT + random::rand_real(0, INTERNAL_HEIGHT)};
        p.vel = rand_vec2(-0.5, 0.5, -1, -0.5) * 16.f;
    }
}

void st_play::try_move_player(direction d)
{
    if (try_move_ent(wor, wor.player(), d))
    {
        ++steps;
        // check for portal if move is successful
        point offs = offset(d);
        entity* p = wor.portal_at(wor.player().tile_x() + offs.x, wor.player().tile_y() + offs.y);
        if (p)
        {
            begin_map_transition(p->portal_state.exit_map, p->portal_state.exit_name);
        }
        else
        {
            // yes this is absolutely evil but SHIP IT
            if (encounters_enabled && wor.has_encounters() && steps >= 8 && random::chance(0.03f))
            {
                begin_battle_transition(pick_random_map_encounter());
                return;
            }
        }
    }
}

void st_interact_context::say(const std::string& message)
{
    owner->show_message(message);
}

entity* st_interact_context::self() const
{
    return ent;
}

void st_interact_context::play_sound(const char* name)
{
    const std::string filename = fmt::format("assets/sound/{}.ogg", name);
    owner->state->audio->play_sound(filename.c_str());
}

entity* st_interact_context::get_entity(const char* name)
{
    return owner->wor.find_entity(name);
}

void st_interact_context::set_flag(const char* name, int value)
{
    owner->flags[name] = value;
}

int st_interact_context::get_flag(const char* name)
{
    if (auto it = owner->flags.find(name); it != owner->flags.end())
    {
        return it->second;
    }
    else
    {
        return 0;
    }
}

void st_interact_context::schedule(uint32_t id, double seconds_from_now)
{
    scheduled_script script;
    script.script_id = id;
    script.t = owner->owner->create_timer(seconds_from_now);
    owner->scheduled_scripts.push_back(script);
}

session_state* st_interact_context::session()
{
    return owner->state->session;
}

void st_interact_context::set_encounter_state(bool enabled)
{
    owner->encounters_enabled = enabled;
}
