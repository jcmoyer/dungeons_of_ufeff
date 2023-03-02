#include "st_battlestats.hpp"
#include "game.hpp"
#include "mathutil.hpp"
#include "ui.hpp"

st_battlestats::st_battlestats(game* g, shared_state* s) : owner{ g }, state{ s } {
}

void st_battlestats::init() {
    exp_gained = 0;
}

void st_battlestats::update() {
    if (sub == fade_in) {
        if (fade_timer.expired(state->frame_counter)) {
            fade_timer = owner->create_timer(3);
            sub = tally;
        }
    } else if (sub == tally) {
        const double k = clamp(fade_timer.progress(state->frame_counter), 0.0, 1.0);
        last_tally_stats = tally_stats;
        tally_stats = old_stats_snapshot;
        tally_stats.add_exp((int)lerp<double>(0, exp_gained, k));

        if (tally_stats != last_tally_stats) {
            state->audio->play_sound("assets/sound/blip.ogg");
        }

        if (tally_stats.level != last_tally_stats.level) {
            state->audio->play_sound("assets/sound/levelup.ogg");
        }
    } else if (sub == fade_out) {
        if (fade_timer.expired(state->frame_counter)) {
            owner->transition(transition_to::play);
        }
    }
}

void st_battlestats::render(double a) {
    //glClearColor(0x14 / 255.f,
    //0x0c / 255.f, 0x1c / 255.f, 1.0f);
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const auto* tex = state->texman->get("assets/amalgamation.png");

    glBindTexture(GL_TEXTURE_2D, tex->tex);
    state->font->set_texture(tex);
    state->font->begin(state->batch);

    constexpr std::string_view TITLE = "Battle Results";
    auto measure = state->font->measure_string(TITLE, glyph_map_font_yellow_large::instance());
    int dx = INTERNAL_WIDTH / 2 - measure.width / 2;

    state->font->draw_string(glyph_map_font_yellow_large::instance(), TITLE, dx, 32);
    state->font->end();

    auto draw_string_centered_x = [&](std::string_view text, int y) {
        auto measure = state->font->measure_string(text, glyph_map_font_white_small::instance());
        const int X = INTERNAL_WIDTH / 2 - measure.width / 2;
        const int Y = y;
        state->font->draw_string(glyph_map_font_white_small::instance(), text, X, Y);
    };

    std::string level_string = fmt::format("Level {}", tally_stats.level);
    std::string life_string = fmt::format("Life {}", tally_stats.max_life());
    std::string power_string = fmt::format("Power {}", tally_stats.power());

    const int EXP_BAR_W = 200;
    const int EXP_BAR_H = 40;
    const int EXP_BAR_X = INTERNAL_WIDTH / 2 - EXP_BAR_W / 2;
    const int EXP_BAR_Y = INTERNAL_HEIGHT / 2 + 10;
    const int LEVEL_Y = INTERNAL_HEIGHT / 2 - 10;
    
    state->font->begin(state->batch);
    draw_string_centered_x(level_string, LEVEL_Y);
    draw_string_centered_x(life_string, EXP_BAR_Y + EXP_BAR_H + 10);
    draw_string_centered_x(power_string, EXP_BAR_Y + EXP_BAR_H + 10 + 10);
    state->font->end();

    const rectangle EXP_BAR_RECT{ EXP_BAR_X, EXP_BAR_Y, EXP_BAR_W, EXP_BAR_H };
    std::string exp_string = fmt::format("EXP {}/{}", tally_stats.player_exp, tally_stats.exp_tnl());

    render_bar(*state->batch, *state->font, *state->quad_render, EXP_BAR_RECT, exp_string, 0xd27d2cff, tally_stats.player_exp / (double)tally_stats.exp_tnl());

    render_fade();
}

void st_battlestats::handle_event(const SDL_Event& ev) {
    if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_x) {
        if (sub != fade_out) {
            sub = fade_out;
            fade_timer = owner->create_timer(1);
        }
    }
}

void st_battlestats::render_fade() {
    if (!(sub == fade_in || sub == fade_out)) {
        return;
    }

    const double out = clamp(fade_timer.progress(state->frame_counter), 0.0, 1.0);
    const double in = 1.0 - out;

    state->quad_render->begin();
    state->quad_render->draw_quad({ 0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT }, 0, 0, 0, sub == fade_in ? in : out);
    state->quad_render->end();
}

void st_battlestats::enter() {
    state->audio->play_sound("assets/sound/fanfare.ogg");

    old_stats_snapshot = state->session->stats;
    tally_stats = old_stats_snapshot;

    sub = fade_in;
    fade_timer = owner->create_timer(1);
}

void st_battlestats::leave() {
    state->session->stats.add_exp(exp_gained);
}

void st_battlestats::set_exp_gained(int32_t amt) {
    exp_gained = amt;
}