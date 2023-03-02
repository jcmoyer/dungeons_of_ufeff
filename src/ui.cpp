#include "ui.hpp"
#include <fmt/format.h>

void render_bar(spritebatch& batch, bmfont& font, quad_renderer& quad, const rectangle& dest, std::string_view text, uint32_t bar_color, double value) {
    //int x = 8, y = 8;

    int x = dest.x;
    int y = dest.y;
    int w = dest.w;
    int h = dest.h;

    auto measure = font.measure_string(text, glyph_map_font_white_small::instance());

    quad.begin();
    quad.draw_quad(dest, 0x140c1cff);

    // 2px margin either end
    const int bar_width = value * (w - 2);

    quad.draw_quad(rectangle{ x + 1, y + 1, bar_width, h - 2 }, bar_color);
    quad.end();

    font.begin(&batch);
    font.draw_string(glyph_map_font_white_small::instance(), text, x + w / 2 - measure.width / 2, y + h / 2 - measure.height / 2, 1, 1, 1);
    font.end();
}
