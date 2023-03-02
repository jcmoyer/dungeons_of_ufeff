#include "st_gameover.hpp"

#include "game.hpp"
#include "mathutil.hpp"

st_gameover::st_gameover(game* g, shared_state* s)
    : owner{g}, state{s}
{
}

void st_gameover::init()
{
}

void st_gameover::update()
{
}

void st_gameover::render(double a)
{
    (void)a;

    const auto* tex = state->texman->get("assets/amalgamation.png");

    glBindTexture(GL_TEXTURE_2D, tex->tex);
    state->font->set_texture(tex);
    state->font->begin(state->batch);
    auto measure = state->font->measure_string("You Died", glyph_map_font_yellow_large::instance());
    int dx = INTERNAL_WIDTH / 2 - measure.width / 2;
    int dy = INTERNAL_HEIGHT / 2 - measure.height / 2;

    state->font->draw_string(glyph_map_font_yellow_large::instance(), "You Died", dx, dy);
    state->font->end();

    render_fade();
}

void st_gameover::handle_event(const SDL_Event& ev)
{
    (void)ev;
}

void st_gameover::render_fade()
{
    if (sub != fade_in)
    {
        return;
    }

    const float c = 1.0f - clamp((float)fade_timer.progress(state->frame_counter), 0.0f, 1.0f);

    state->quad_render->begin();
    state->quad_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT}, 0, 0, 0, c);
    state->quad_render->end();
}

void st_gameover::enter(gamestate* old)
{
    (void)old;

    sub = fade_in;
    fade_timer = owner->create_timer(3);
    state->audio->play_sound("assets/sound/gameover.ogg");
}

void st_gameover::leave()
{
}
