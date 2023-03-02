#pragma once

#include <string_view>

#include "bmfont.hpp"
#include "quad_renderer.hpp"
#include "spritebatch.hpp"

void render_bar(spritebatch& batch, bmfont& font, quad_renderer& quad, const rectangle& dest, std::string_view text, uint32_t bar_color, double value);
void render_button(spritebatch& batch, bmfont& font, quad_renderer& quad, const rectangle& dest, std::string_view text, bool pressed);
