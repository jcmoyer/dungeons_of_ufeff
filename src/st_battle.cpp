#include "st_battle.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>

#include "animation_data.hpp"
#include "audio.hpp"
#include "battle/encounters.hpp"
#include "game.hpp"
#include "mathutil.hpp"
#include "obj_loader.hpp"
#include "random_vec.hpp"
#include "rectangle.hpp"
#include "texture_manager.hpp"
#include "world.hpp"

bool should_sprite_flip(direction d)
{
    return d == left;
}

void st_battle::begin_transition()
{
    b_fade_timer = owner->create_timer(1);
    sub = battle_fadeout;
}

st_battle::st_battle(game* o, shared_state* s)
    : owner{o}, state{s}
{
}

sk_flash_jump s_flash_jump;
sk_double_throw s_double_throw;
sk_avenger s_avenger;

sk_double_throw s_shadow_double_throw;
sk_avenger s_shadow_avenger;
skill* s_shadow_echo = nullptr;
timer s_shadow_timer;

void load_battle_field_mesh(const battle_field_properties& info, battle_field_mesh& mesh, battle_field_bounds& bounds)
{
    const std::string filename = fmt::format("assets/models/{}.bin", info.name);
    std::ifstream input(filename, std::ios::binary);

    uint32_t nverts = 0;
    input.read((char*)&nverts, sizeof(uint32_t));

    mesh.vertices.resize(nverts);
    input.read((char*)mesh.vertices.data(), nverts * sizeof(battle_field_vertex));

    input.read((char*)&bounds.left, sizeof(float));
    input.read((char*)&bounds.right, sizeof(float));
    input.read((char*)&bounds.floor, sizeof(float));

    // bit of a hack; battle object origin is at their midpoint and are 16 units wide and tall
    // we adjust the bounds so that they don't "clip" outside the playable area drawn in blender
    bounds.left += 8.f;
    bounds.right -= 8.f;
    bounds.floor += 8.f;

    for (auto& v : mesh.vertices)
    {
        // fixup texcoords
        v.v = 1.0f - v.v;
    }
}

cached_mesh& st_battle::get_mesh(const char* name)
{
    if (auto it = meshes.find(name); it != meshes.end())
    {
        return it->second;
    }
    else
    {
        cached_mesh cmesh;
        load_battle_field_mesh(*bf_props, cmesh.mesh, cmesh.bounds);
        meshes.emplace(name, std::move(cmesh));
        return meshes.at(name);
    }
}

void st_battle::init()
{
    g_audio = state->audio;
}

void st_battle::update()
{
    if (sub == battle_fadein)
    {
        if (b_fade_timer.expired(state->frame_counter))
        {
            b_cam.tracking_rate = b_cam.TRACK_RATE_NORMAL;
            sub = none;
        }
    }
    else if (sub == battle_fadeout)
    {
        if (b_fade_timer.expired(state->frame_counter))
        {
            current_music->volume = 0;
            current_music->done = true;
            if (b_result == win)
            {
                owner->get_battlestats_state().set_exp_gained(b_field.calc_exp_value());
                owner->transition(transition_to::battlestats);
            }
            else
            {
                owner->transition(transition_to::gameover);
            }
            return;
        }
        else
        {
            current_music->volume = 1.0f - static_cast<float>(b_fade_timer.progress(state->frame_counter));
        }
    }

    b_field.update(sub == battle_fadein);

    if (sub == none)
    {
        if (b_field.player().can_act())
        {
            if (state->input->keys[SDLK_LEFT])
            {
                b_field.player().move_left();
            }
            else if (state->input->keys[SDLK_RIGHT])
            {
                b_field.player().move_right();
            }
            else
            {
                // anim.set_animation("down");
            }
        }
    }

    s_flash_jump.update(b_field, b_field.player());
    s_double_throw.update(b_field, b_field.player());
    s_avenger.update(b_field, b_field.player());

    s_shadow_double_throw.update(b_field, b_field.player());
    s_shadow_avenger.update(b_field, b_field.player());

    if (s_shadow_echo && s_shadow_timer.expired(state->frame_counter))
    {
        b_field.player().use_skill_unconditionally(b_field, s_shadow_echo);
        s_shadow_echo = nullptr;
    }

    if (sub == none || sub == battle_fadeout)
    {
        b_cam.set_target(b_field.player().pos + glm::vec2(0, 8));
    }

    b_cam.update();

    if (sub == none && !b_field.hostiles_alive())
    {
        b_result = win;
        begin_transition();
    }
    else if (sub == none && !b_field.player().alive)
    {
        b_result = lose;
        begin_transition();
    }
}

void st_battle::render(double a)
{
    glEnable(GL_DEPTH_TEST);

    glClearColor(bf_props->sky_color.r, bf_props->sky_color.g, bf_props->sky_color.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3d modo engage
    glm::mat4 view = b_cam.get_view(a);
    glm::mat4 proj = glm::perspective(3.141f / 4.f, 512.f / 256.f, 1.f, 10000.f);

    const auto* tex = state->texman->get("assets/amalgamation.png");
    glBindTexture(GL_TEXTURE_2D, tex->tex);

    bf_render.set_light_direction(bf_props->light_direction);
    bf_render.render(view, proj);

    bo_render.begin(view, proj);

    glDepthMask(GL_FALSE);

    for (auto& c : b_field.characters)
    {
        bool flash = false;

        if (c.hitstun_frames > 0)
        {
            // kinda dirty but w/e
            if (c.info == &B_CHARINFO[BCI_PLAYER])
            {
                flash = c.hitstun_frames % 4 == 0;
            }
            else
            {
                flash = true;
            }
        }

        auto interp_rect = c.worldspace_interp_rect(a);
        bo_render.draw_quad(tex, c.anim.current_rect(), interp_rect.x, interp_rect.y, interp_rect.w, interp_rect.h, flash);

        if (c.info == &B_CHARINFO[BCI_PLAYER] && state->session->has_shadowpartner)
        {
            if (c.facing == left)
            {
                bo_render.draw_quad(tex, c.anim.current_rect(), interp_rect.x + 8, interp_rect.y, interp_rect.w, interp_rect.h, flash, false, 0.1f);
            }
            else
            {
                bo_render.draw_quad(tex, c.anim.current_rect(), interp_rect.x - 8, interp_rect.y, interp_rect.w, interp_rect.h, flash, false, 0.1f);
            }
        }
    }

    for (auto& p : b_field.projectiles)
    {
        auto interp_rect = p.worldspace_interp_rect(a);
        bo_render.draw_quad_rotated(tex, p.anim.current_rect(), interp_rect.x, interp_rect.y, interp_rect.w, interp_rect.h, p.interp_angle(a), false);
    }

    for (auto& fx : b_field.fx_sys.effects)
    {
        auto interp_rect = fx.worldspace_interp_rect(a);

        if (fx.attached())
        {
            auto owner_rect = b_field.characters[fx.owner].worldspace_interp_rect(a);
            interp_rect.x += owner_rect.x + owner_rect.w / 2;
            interp_rect.y += owner_rect.y + owner_rect.h / 2;
        }

        bo_render.draw_quad(tex, fx.anim.current_rect(), interp_rect.x, interp_rect.y, interp_rect.w, interp_rect.h, 0, should_sprite_flip(fx.dir));
    }

    for (auto& particle : b_field.particle_sys.particles)
    {
        auto interp_rect = particle.worldspace_interp_rect(a);
        bo_render.draw_quad(tex, particle.anim.current_rect(), interp_rect.x, interp_rect.y, interp_rect.w, interp_rect.h, 0);
    }

    bo_render.end();

    glDepthMask(GL_TRUE);

    // back to 2d mode for fade/overlays
    glDisable(GL_DEPTH_TEST);

    render_health_bar();
    render_fade();
}

#include "ui.hpp"
void st_battle::render_health_bar()
{
    constexpr int TOTAL_HEALTH_BAR_WIDTH = 80;
    constexpr int TOTAL_HEALTH_BAR_HEIGHT = 20;

    std::string life = fmt::format("Life: {}/{}", b_field.player().life, state->session->stats.max_life());
    const rectangle player_health_bar{8, 8, TOTAL_HEALTH_BAR_WIDTH, TOTAL_HEALTH_BAR_HEIGHT};
    const rectangle enemy_health_bar{INTERNAL_WIDTH - TOTAL_HEALTH_BAR_WIDTH - 8, 8, TOTAL_HEALTH_BAR_WIDTH, TOTAL_HEALTH_BAR_HEIGHT};

    render_bar(*state->batch, *state->font, *state->quad_render, player_health_bar, life, 0xd04648ff, b_field.player().life / (double)state->session->stats.max_life());

    if (b_field.last_enemy_hit != SIZE_MAX)
    {
        const int32_t e_life = b_field.characters[b_field.last_enemy_hit].life;
        const int32_t e_max_life = b_field.characters[b_field.last_enemy_hit].info->max_life;

        std::string enemy_life = fmt::format("Enemy: {}/{}", b_field.characters[b_field.last_enemy_hit].life, b_field.characters[b_field.last_enemy_hit].info->max_life);
        render_bar(*state->batch, *state->font, *state->quad_render, enemy_health_bar, enemy_life, 0xd04648ff, e_life / (double)e_max_life);
    }
}

void st_battle::render_fade()
{
    if (!(sub == battle_fadein || sub == battle_fadeout))
    {
        return;
    }

    const float p = clamp(static_cast<float>(b_fade_timer.progress(state->frame_counter)), 0.0f, 1.0f);

    const float in = 1.0f - p;
    const float out = p;

    state->quad_render->begin();
    state->quad_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT}, 0, 0, 0, sub == battle_fadein ? in : out);
    state->quad_render->end();
}

void st_battle::handle_event(const SDL_Event& ev)
{
    if (sub == none)
    {
        if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
        {
            owner->transition(transition_to::options);
        }

        if (b_field.player().can_act())
        {
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == state->session->keybinds.get_key(IA_JUMP))
            {
                b_field.player().jump();
                if (state->session->has_flashjump)
                {
                    b_field.player().use_skill(b_field, &s_flash_jump);
                }
            }

            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == state->session->keybinds.get_key(IA_SKILL1))
            {
                b_field.player().use_skill(b_field, &s_double_throw);

                if (state->session->has_shadowpartner)
                {
                    s_shadow_timer = owner->create_timer(0.1);
                    s_shadow_echo = &s_shadow_double_throw;
                }
            }

            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == state->session->keybinds.get_key(IA_SKILL2))
            {
                if (state->session->has_avenger)
                {
                    b_field.player().use_skill(b_field, &s_avenger);

                    if (state->session->has_shadowpartner)
                    {
                        s_shadow_timer = owner->create_timer(0.1);
                        s_shadow_echo = &s_shadow_avenger;
                    }
                }
            }
        }
    }
}

void st_battle::enter(gamestate* old)
{
    if (old == &owner->get_options_state())
    {
        // do nothing
        return;
    }

    sub = battle_fadein;
    b_fade_timer = owner->create_timer(3);

    std::string music_filename = fmt::format("assets/music/{}.ogg", "make_your_stand");
    current_music = state->audio->play_music(music_filename.c_str());

    // extra cuteness
    b_cam.tracking_rate = b_cam.TRACK_RATE_OPENING;
    // b_cam.center_on({ 1200, 400 });

    b_field.clear();

    if (bf_name == "bf_dungeon")
    {
        bf_props = &BF_DUNGEON;
    }
    else if (bf_name == "bf_sanctum")
    {
        bf_props = &BF_SANCTUM;
    }
    else if (bf_name == "bf_lair")
    {
        bf_props = &BF_LAIR;
    }
    else
    {
        assert(false && "invalid battle field name");
        bf_props = &BF_DUNGEON;
    }

    // probably rewrite this to use actual gl storage to prevent re-uploading
    cached_mesh& cmesh = get_mesh(bf_props->name);
    bf_render.set_mesh(cmesh.mesh);
    b_field.bounds = cmesh.bounds;

    b_field.spawn_character(BCI_PLAYER, {b_field.bounds.left + 16, 0});

    int32_t spawn_offset = -16;
    for (size_t i = 0; i < 8; ++i)
    {
        if (enc.enemies[i] == BCI_NULL)
        {
            continue;
        }
        b_field.spawn_character(enc.enemies[i], {b_field.bounds.right + spawn_offset, 0});
        spawn_offset -= 32;
    }

    b_field.set_player_index(0);
    b_field.player().life = state->session->stats.max_life();
    b_field.player().power = state->session->stats.power();

    // tick twice to move objects into position... don't ask
    b_field.init_positions();
    b_cam.center_on(b_field.characters[b_field.characters.size() - 1].pos + glm::vec2(0, 8));
    b_cam.set_target(b_field.characters[b_field.characters.size() - 1].pos + glm::vec2(0, 8));
}

void st_battle::leave()
{
}

void st_battle::set_battle_field_name(std::string bf_name_)
{
    bf_name = std::move(bf_name_);
}

void st_battle::set_encounter(encounter enc_)
{
    enc = enc_;
}
