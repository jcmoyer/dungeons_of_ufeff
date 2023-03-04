#include "st_options.hpp"

#include "game.hpp"
#include "mathutil.hpp"
#include "ui.hpp"

st_options::st_options(game* g, shared_state* s)
    : owner{g}, state{s}
{
}

void st_options::init()
{
    input_buttons.emplace_back(IA_INTERACT);
    input_buttons.emplace_back(IA_JUMP);
    input_buttons.emplace_back(IA_SKILL1);
    input_buttons.emplace_back(IA_SKILL2);

    int n = 2;
    int fact = INTERNAL_HEIGHT / 6;
    for (option_button& b : input_buttons)
    {
        b.update_text(state->session->keybinds);
        b.rect.w = 100;
        b.rect.h = 25;
        b.rect.x = INTERNAL_WIDTH / 4 - b.rect.w / 2;
        b.rect.y = fact * n - b.rect.h / 2;
        ++n;
    }

    res_button.rect.w = 100;
    res_button.rect.h = 25;
    res_button.rect.x = 3 * INTERNAL_WIDTH / 4 - res_button.rect.w / 2;
    res_button.rect.y = fact * 2 - res_button.rect.h / 2;
    res_button.update_text(owner->get_scale());

    fs_button.rect.w = 100;
    fs_button.rect.h = 25;
    fs_button.rect.x = 3 * INTERNAL_WIDTH / 4 - fs_button.rect.w / 2;
    fs_button.rect.y = fact * 3 - fs_button.rect.h / 2;
}

void st_options::update()
{
}

void st_options::render(double a)
{
    (void)a;

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    for (option_button& b : input_buttons)
    {
        render_button(*state->batch, *state->font, *state->quad_render, b.rect, b.text, &b == pending_button);
    }

    render_button(*state->batch, *state->font, *state->quad_render, res_button.rect, res_button.text, false);
    render_button(*state->batch, *state->font, *state->quad_render, fs_button.rect, fs_button.text, false);

    state->font->begin(state->batch);

    if (pending_button)
    {
        rectangle dest = pending_button->rect;
        dest.inflate(2, 2);
        dest.translate(4 + dest.w, 0);
        draw_string_centered(*state->font, glyph_map_font_white_small::instance(), "Waiting for key...", dest, 1, 1, 1);
    }

    draw_string_centered(*state->font, glyph_map_font_yellow_large::instance(), "Controls", {0, 0, INTERNAL_WIDTH / 2, input_buttons[0].rect.y}, 1, 1, 1);
    draw_string_centered(*state->font, glyph_map_font_yellow_large::instance(), "Graphics", {INTERNAL_WIDTH / 2, 0, INTERNAL_WIDTH / 2, input_buttons[0].rect.y}, 1, 1, 1);

    state->font->end();
}

void st_options::handle_event(const SDL_Event& ev)
{
    if (sub == waiting_for_key && ev.type == SDL_KEYDOWN)
    {
        state->session->keybinds.rebind_key(pending_button->act, ev.key.keysym.sym);
        // as rebind_key may affect multiple binds, we need to update the text on all buttons
        for (option_button& b : input_buttons)
            b.update_text(state->session->keybinds);
        pending_button = nullptr;
        sub = none;
    }

    if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT)
    {
        glm::vec2 cursor = owner->unproject({ev.button.x, ev.button.y});

        for (option_button& b : input_buttons)
        {
            if (b.rect.contains((int)cursor.x, (int)cursor.y))
            {
                pending_button = &b;
                sub = waiting_for_key;
                return;
            }
        }

        // user clicked somewhere, but not on a rebind button; cancel the rebind operation
        pending_button = nullptr;
        sub = none;

        if (res_button.rect.contains((int)cursor.x, (int)cursor.y))
        {
            owner->select_next_resolution();
            res_button.update_text(owner->get_scale());
        }
        else if (fs_button.rect.contains((int)cursor.x, (int)cursor.y))
        {
            owner->toggle_fullscreen();
        }
    }

    if (sub == none && ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
    {
        owner->transition(previous_state);
    }
}

void st_options::enter(gamestate* old)
{
    previous_state = old;
}

void st_options::leave()
{
}

void option_button::update_text(const action_map& m)
{
    SDL_Keycode key = m.get_key(act);
    text = fmt::format("{} ({})", get_action_name(act), SDL_GetKeyName(key));
}

void resolution_button::update_text(int scale)
{
    text = fmt::format("Resolution {}x", scale);
}