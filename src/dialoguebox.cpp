#include "dialoguebox.hpp"

#include "bmfont.hpp"
#include "spritebatch.hpp"
#include "texture_manager.hpp"

const int PADDING = 2;

struct dialogue_computed_rects
{
    rectangle top_left;
    rectangle top_right;
    rectangle bottom_left;
    rectangle bottom_right;
    rectangle top;
    rectangle bottom;
    rectangle left;
    rectangle right;
    rectangle fill;

    void translate_all(int dx, int dy)
    {
        top_left.translate(dx, dy);
        top_right.translate(dx, dy);
        bottom_left.translate(dx, dy);
        bottom_right.translate(dx, dy);
        top.translate(dx, dy);
        bottom.translate(dx, dy);
        left.translate(dx, dy);
        right.translate(dx, dy);
        fill.translate(dx, dy);
    }
};

dialogue_computed_rects compute_dialogue_rects(const dialogue_info& di, std::string_view text)
{
    bmfont font;
    auto measurement = font.measure_string(text, glyph_map_font_white_small::instance());
    rectangle text_rect{0, 0, measurement.width, measurement.height};

    const auto TL = DIALOGUE_RECTS[DIA_TOPLEFT];
    const auto T = DIALOGUE_RECTS[DIA_TOP];
    const auto TR = DIALOGUE_RECTS[DIA_TOPRIGHT];
    const auto BL = DIALOGUE_RECTS[DIA_BOTTOMLEFT];
    const auto B = DIALOGUE_RECTS[DIA_BOTTOM];
    const auto BR = DIALOGUE_RECTS[DIA_BOTTOMRIGHT];
    const auto L = DIALOGUE_RECTS[DIA_LEFT];
    const auto R = DIALOGUE_RECTS[DIA_RIGHT];

    // add padding and move into place
    text_rect.inflate(PADDING, PADDING);
    text_rect.x = TL.w;
    text_rect.y = TL.h;

    dialogue_computed_rects rects;

    rects.top_left = {0, 0, TL.w, TL.h};
    rects.top = {TL.w, 0, text_rect.w, T.h};
    rects.top_right = {TL.w + text_rect.w, 0, TR.w, TR.h};
    rects.bottom_left = {0, T.h + text_rect.h, BL.w, BL.h};
    rects.bottom = {TL.w, T.h + text_rect.h, text_rect.w, B.h};
    rects.bottom_right = {TL.w + text_rect.w, T.h + text_rect.h, BR.w, BR.h};
    rects.left = {0, T.h, L.w, text_rect.h};
    rects.right = {L.w + text_rect.w, T.h, R.w, text_rect.h};
    rects.fill = text_rect;

    return rects;
}

rectangle measure_dialogue_box(const dialogue_info& di, std::string_view text)
{
    auto rects = compute_dialogue_rects(di, text);
    return {0, 0, rects.left.w + rects.fill.w + rects.right.w, rects.top.h + rects.fill.h + rects.bottom.h};
}

void render_dialogue_box(const dialogue_info& di, spritebatch& batch, std::string_view text, int x, int y)
{
    bmfont font;
    font.set_texture(di.sides); // TODO: font won't always necessarily come from "sides" texture

    auto rects = compute_dialogue_rects(di, text);

    rects.translate_all(x, y);

    glBindTexture(GL_TEXTURE_2D, di.sides->tex);
    batch.begin();
    batch.draw_quad(di.sides, DIALOGUE_RECTS[DIA_TOPLEFT], rects.top_left);
    batch.draw_quad(di.sides, DIALOGUE_RECTS[DIA_TOPRIGHT], rects.top_right);
    batch.draw_quad(di.sides, DIALOGUE_RECTS[DIA_TOP], rects.top);
    batch.draw_quad(di.sides, DIALOGUE_RECTS[DIA_BOTTOMLEFT], rects.bottom_left);
    batch.draw_quad(di.sides, DIALOGUE_RECTS[DIA_BOTTOM], rects.bottom);
    batch.draw_quad(di.sides, DIALOGUE_RECTS[DIA_BOTTOMRIGHT], rects.bottom_right);
    batch.draw_quad(di.sides, DIALOGUE_RECTS[DIA_LEFT], rects.left);
    batch.draw_quad(di.sides, DIALOGUE_RECTS[DIA_RIGHT], rects.right);
    batch.end();

    glBindTexture(GL_TEXTURE_2D, di.fill->tex);
    batch.begin();
    const float RX = rects.fill.w / (float)di.fill->width;
    const float RY = rects.fill.h / (float)di.fill->height;
    batch.draw_tiled_quad(di.fill, rects.fill, RX, RY);
    batch.end();

    font.begin(&batch);
    font.draw_string(glyph_map_font_white_small::instance(), text, rects.fill.x + PADDING, rects.fill.y + PADDING);
    font.end();
}