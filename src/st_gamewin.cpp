#include "st_gamewin.hpp"
#include "game.hpp"
#include "mathutil.hpp"

st_gamewin::st_gamewin(game* g, shared_state* s) : owner{ g }, state{ s } {
}

void st_gamewin::init() {

}

void st_gamewin::update() {
}

void st_gamewin::render(double a) {
    const auto* tex = state->texman->get("assets/amalgamation.png");

    glBindTexture(GL_TEXTURE_2D, tex->tex);
    state->font->set_texture(tex);
    state->font->begin(state->batch);

    const char* text = "Congratulations";
    auto measure = state->font->measure_string(text, glyph_map_font_yellow_large::instance());
    int dx = INTERNAL_WIDTH / 2 - measure.width / 2;
    int dy = INTERNAL_HEIGHT / 2 - measure.height;
    state->font->draw_string(glyph_map_font_yellow_large::instance(), text, dx, dy);


    text = "You killed Ragworm, freeing UFEFF.";
    measure = state->font->measure_string(text, glyph_map_font_white_small::instance());
    dx = INTERNAL_WIDTH / 2 - measure.width / 2;
    dy = INTERNAL_HEIGHT / 2 + measure.height;
    state->font->draw_string(glyph_map_font_white_small::instance(), text, dx, dy);

    text = "Dungeons of UFEFF, a game by jcmoyer for ufeffjam 1. Thanks for playing!";
    measure = state->font->measure_string(text, glyph_map_font_white_small::instance());
    dx = INTERNAL_WIDTH / 2 - measure.width / 2;
    dy = INTERNAL_HEIGHT - measure.height;
    state->font->draw_string(glyph_map_font_white_small::instance(), text, dx, dy);

    state->font->end();

    render_fade();
}

void st_gamewin::handle_event(const SDL_Event& ev) {
}

void st_gamewin::render_fade() {
    if (sub != fade_in) {
        return;
    }

    const double c = 1.0 - clamp(fade_timer.progress(state->frame_counter), 0.0, 1.0);

    state->quad_render->begin();
    state->quad_render->draw_quad({ 0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT }, 0, 0, 0, c);
    state->quad_render->end();
}

void st_gamewin::enter() {
    sub = fade_in;
    fade_timer = owner->create_timer(3);
    state->audio->play_sound("assets/sound/torch.ogg");
}

void st_gamewin::leave() {
}
