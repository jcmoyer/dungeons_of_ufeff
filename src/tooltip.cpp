#include "tooltip.hpp"
#include "imm_renderer.hpp"
#include "texture_manager.hpp"

constexpr rectangle rects[9] = {
    {0, 0, 3, 3},
    {3, 0, 3, 3},
    {29, 0, 3, 3},

    {0, 3, 3, 3},
    {3, 3, 3, 3},
    {29, 3, 3, 3},

    {0, 29, 3, 3},
    {3, 29, 3, 3},
    {29, 29, 3, 3}
};

void tooltip::draw(imm_renderer* renderer, const SDL_Rect& dest) {
    glBindTexture(GL_TEXTURE_2D, tex->tex);

    // == ROW 1 ==
    rectangle dest_00;
    dest_00.x = dest.x;
    dest_00.y = dest.y;
    dest_00.w = 3;
    dest_00.h = 3;

    rectangle dest_10;
    dest_10.x = dest.x + 3;
    dest_10.y = dest.y;
    dest_10.w = dest.w - 6;
    dest_10.h = 3;

    rectangle dest_20;
    dest_20.x = dest.x + dest_00.w + dest_10.w;
    dest_20.y = dest.y;
    dest_20.w = 3;
    dest_20.h = 3;

    // == ROW 2 ==
    rectangle dest_01;
    dest_01.x = dest.x;
    dest_01.y = dest.y + 3;
    dest_01.w = 3;
    dest_01.h = dest.h - 6;

    rectangle dest_11;
    dest_11.x = dest.x + 3;
    dest_11.y = dest.y + 3;
    dest_11.w = dest.w - 6;
    dest_11.h = dest.h - 6;

    rectangle dest_21;
    dest_21.x = dest.x + dest_00.w + dest_10.w;
    dest_21.y = dest.y + 3;
    dest_21.w = 3;
    dest_21.h = dest.h - 6;

    // == ROW 3 ==
    rectangle dest_02;
    dest_02.x = dest.x;
    dest_02.y = dest.y + dest_00.h + dest_01.h;
    dest_02.w = 3;
    dest_02.h = 3;

    rectangle dest_12;
    dest_12.x = dest.x + 3;
    dest_12.y = dest.y + dest_00.h + dest_01.h;
    dest_12.w = dest.w - 6;
    dest_12.h = 3;

    rectangle dest_22;
    dest_22.x = dest.x + dest_00.w + dest_10.w;
    dest_22.y = dest.y + dest_00.h + dest_01.h;
    dest_22.w = 3;
    dest_22.h = 3;

    renderer->begin();
    renderer->draw_quad(tex, rects[0], dest_00);
    renderer->draw_quad(tex, rects[1], dest_10);
    renderer->draw_quad(tex, rects[2], dest_20);
    renderer->draw_quad(tex, rects[3], dest_01);
    renderer->draw_quad(tex, rects[4], dest_11);
    renderer->draw_quad(tex, rects[5], dest_21);
    renderer->draw_quad(tex, rects[6], dest_02);
    renderer->draw_quad(tex, rects[7], dest_12);
    renderer->draw_quad(tex, rects[8], dest_22);
}