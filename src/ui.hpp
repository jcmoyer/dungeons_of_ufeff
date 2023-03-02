#pragma once

#include "bmfont.hpp"
#include "quad_renderer.hpp"
#include "spritebatch.hpp"
#include <string_view>

void render_bar(spritebatch& batch, bmfont& font, quad_renderer& quad, const rectangle& dest, std::string_view text, uint32_t bar_color, double value);

