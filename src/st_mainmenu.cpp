#include "st_mainmenu.hpp"

#include "game.hpp"
#include "mathutil.hpp"

st_mainmenu::st_mainmenu(game* g, shared_state* s)
    : owner{g}, state{s}
{
}

void st_mainmenu::init()
{
}

void st_mainmenu::update()
{
    if (sub == fade_to_game && fade_timer.expired(state->frame_counter))
    {
        owner->transition(transition_to::play);
    }
}

void st_mainmenu::render(double a)
{
    (void)a;

    const auto* tex = state->texman->get("assets/amalgamation.png");

    glBindTexture(GL_TEXTURE_2D, tex->tex);

    int offset = -(int)(state->frame_counter % 16);
    int cycles = state->frame_counter / 16;

    const int BLINK_PERIOD = 32;
    bool visible = (state->frame_counter % BLINK_PERIOD) < (BLINK_PERIOD / 2);

    state->batch->begin();
    for (int y = 0; y < INTERNAL_HEIGHT + 16; y += 16)
    {
        for (int x = 0; x < INTERNAL_WIDTH + 16; x += 16)
        {
            int xx = x + cycles * 16;

            rectangle src = {0, 128, 16, 16};
            if (((xx * xx + y) % 17) == 0)
            {
                src.x += 16;
            }
            rectangle dest{offset + x, y, 16, 16};
            state->batch->draw_quad(tex, src, dest);
        }
    }
    state->batch->end();

    state->font->set_texture(state->texman->get("assets/amalgamation.png"));
    state->font->begin(state->batch);
    auto measure = state->font->measure_string("Dungeons of UFEFF", glyph_map_font_yellow_large::instance());
    int dx = INTERNAL_WIDTH / 2 - measure.width / 2;
    int dy = INTERNAL_HEIGHT / 3 - measure.height / 2;

    state->font->draw_string(glyph_map_font_yellow_large::instance(), "Dungeons of UFEFF", dx, dy);

    if (visible)
    {
        measure = state->font->measure_string("press X to start", glyph_map_font_blue_large::instance());
        dx = INTERNAL_WIDTH / 2 - measure.width / 2;
        state->font->draw_string(glyph_map_font_blue_large::instance(), "press X to start", dx, dy + 64);
    }
    state->font->end();

    render_game_fade();
}

void st_mainmenu::handle_event(const SDL_Event& ev)
{
    if (sub == none)
    {
        if (ev.key.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_x)
        {
            begin_game_transition();
        }
    }
}

void st_mainmenu::begin_game_transition()
{
    sub = fade_to_game;
    fade_timer = owner->create_timer(1);
    state->audio->play_sound("assets/sound/start.ogg");
}

void st_mainmenu::render_game_fade()
{
    if (sub != fade_to_game)
    {
        return;
    }

    const float c = clamp(static_cast<float>(fade_timer.progress(state->frame_counter)), 0.0f, 1.0f);

    state->quad_render->begin();
    state->quad_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT}, 0, 0, 0, c);
    state->quad_render->end();
}

void st_mainmenu::enter(gamestate* old)
{
    (void)old;
}

void st_mainmenu::leave()
{
}
